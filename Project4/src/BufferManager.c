#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "BufferManager.h"
#include "DiskManager.h"
#include "globals.h"

Bufstrt * buffer[MAXTABLE+1];
Bufstrt * bufferlast[MAXTABLE+1];
int bufsize;
int curPos[MAXTABLE+1];
int numOfCol[MAXTABLE+1];

Bufstrt * makeBufNode()
{
	Bufstrt * newBuf = (Bufstrt*)malloc(sizeof(Bufstrt));
	newBuf->table_id = -1;
	newBuf->page_num = -1;
	newBuf->is_dirty = -1;
	newBuf->is_pinned = -1;
	newBuf->next = NULL;
	newBuf->prev = NULL;

	return newBuf;	
}

void bufferAttr(Bufstrt* node,dbint offset, int table_id)
{
	node->page_num = offset;
	node->table_id = table_id;	
	node->is_dirty = 0;
	node->is_pinned = 0;
}

void newBuf(int num_buf, int table_id)
{
	int i;
	bufsize = num_buf;

	buffer[table_id] = NULL;
	bufferlast[table_id] = NULL;
	curPos[table_id] = 0;
}

void closeBuf(int table_id)
{
	Bufstrt * deleteBuf = buffer[table_id];
	Bufstrt * temp = deleteBuf;

	if(temp == NULL)
		return;

	while(temp != NULL)
	{
		file_write_page(get_page_offset(temp), &temp->frame, table_id);
		deleteBuf = temp;
		temp = temp->next;
		free(deleteBuf);
	}		
	buffer[table_id] = bufferlast[table_id] = NULL;
	curPos[table_id] = 0;
}


pagenum_t get_page_offset(Bufstrt * node)
{
	return node->page_num;
}

Bufstrt * find_leaf_buffer(pagenum_t offset, int table_id)
{
	Bufstrt * temp = buffer[table_id];
	
	while(temp != NULL && get_page_offset(temp) != offset)
		temp = temp->next;

	return temp;	
}

Page * set_leaf_page(Page * dest, Page * src)
{
	dest->leafp.pheader.parent_page = src->leafp.pheader.parent_page;
	dest->leafp.pheader.is_leaf = 1; 
	dest->leafp.pheader.numOfKeys = src->leafp.pheader.numOfKeys;
	memset(dest->leafp.pheader.reserved,0,RESERVEDSIZE);
	dest->leafp.pheader.other_page_offset = src->leafp.pheader.other_page_offset;
}

void buf_flush_page(pagenum_t offset, int table_id)
{
	Bufstrt * tempNode = find_leaf_buffer(offset, table_id);
	Bufstrt * oldNode;

	if(tempNode == NULL)
		return;
	else
	{
		if(curPos[table_id] == 1)
		{
			buffer[table_id] = bufferlast[table_id] = NULL;
			if(offset != header[table_id]->headerp.rootp_offset)
				file_free_page(offset, table_id);
			curPos[table_id] = 0;
			free(tempNode);
		}
		else if(curPos[table_id] == 2)
		{
			if(buffer[table_id] == tempNode)
			{
				buffer[table_id] = bufferlast[table_id];
				buffer[table_id]->prev = buffer[table_id]->next = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}
			else
			{
				bufferlast[table_id] = buffer[table_id];
				buffer[table_id]->prev = buffer[table_id]->next = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}			
			curPos[table_id]--;
		}
		else
		{
			if(buffer[table_id] == tempNode)
			{
				buffer[table_id] = buffer[table_id]->next;
				buffer[table_id]->prev = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}
			else if(bufferlast[table_id] == tempNode)
			{
				bufferlast[table_id] = bufferlast[table_id]->prev;
				bufferlast[table_id]->next = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}
			else
			{
				tempNode->next->prev = tempNode->prev;
				tempNode->prev->next = tempNode->next;	
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}
			curPos[table_id]--;
		}
	}
}

Page * buf_read_page(pagenum_t offset, Page * dest ,int table_id)
{
	Bufstrt * tempNode = find_leaf_buffer(offset, table_id);
	Bufstrt * oldNode;

	if(tempNode != NULL)
	{
		buf_lru_set(tempNode,table_id);	
		memcpy(dest, &tempNode->frame, PAGESIZE);
		return &tempNode->frame;	
	}
	else
	{
		if(curPos[table_id]== 0)
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			buffer[table_id] = bufferlast[table_id] = tempNode;
			file_read_page(offset, &buffer[table_id]->frame, table_id);
			curPos[table_id]++;	
		
		}
		else if(bufsize > curPos[table_id])
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			bufferlast[table_id]->next = tempNode;
			tempNode->prev = bufferlast[table_id];
			bufferlast[table_id] = tempNode;
			file_read_page(offset, &bufferlast[table_id]->frame, table_id);	
			bufferlast[table_id]->next = NULL;
			curPos[table_id]++;	
		}
		else
		{
			tempNode = makeBufNode();		
			bufferAttr(tempNode, offset, table_id);
			file_write_page(get_page_offset(buffer[table_id]), &buffer[table_id]->frame, table_id);
			oldNode = buffer[table_id];
			buffer[table_id] = buffer[table_id]->next;
			buffer[table_id]->prev = NULL;
			free(oldNode);
			bufferlast[table_id]->next = tempNode;
			tempNode->prev = bufferlast[table_id];
			bufferlast[table_id] = tempNode;
			file_read_page(offset, &bufferlast[table_id]->frame, table_id);
			bufferlast[table_id]->next = NULL;
		}
	}
	
	memcpy(dest, &bufferlast[table_id]->frame, PAGESIZE);
	return &bufferlast[table_id]->frame;
}

void buf_lru_set(Bufstrt * node, int table_id)
{
	Bufstrt * tempNode = node;
	
	if(tempNode->next == NULL)
		return;

	if(tempNode->prev == NULL)
	{
		tempNode->next->prev = NULL;
		buffer[table_id] = tempNode->next;
		bufferlast[table_id]->next = tempNode;
		tempNode->prev = bufferlast[table_id];
		tempNode->next = NULL;
		bufferlast[table_id] = tempNode;

		return;
	}
	
	tempNode->next->prev = tempNode->prev;
	tempNode->prev->next = tempNode->next;
	
	bufferlast[table_id]->next = tempNode;
	tempNode->prev = bufferlast[table_id];
	tempNode->next = NULL;
	bufferlast[table_id] = tempNode;

	return;
}

void buf_write_page(pagenum_t offset, Page * src ,int table_id)
{
	Bufstrt * tempNode = find_leaf_buffer(offset, table_id);
	Bufstrt * oldNode;		

	if(tempNode != NULL)
	{
		memcpy(&tempNode->frame, src, PAGESIZE);
		tempNode->is_dirty = 1;
		buf_lru_set(tempNode,table_id);	
	}
	else
	{
		if(curPos[table_id] == 0)
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			memcpy(&tempNode->frame, src, PAGESIZE);
			buffer[table_id] = bufferlast[table_id] = tempNode;
			curPos[table_id]++;	
		}
		else if(bufsize > curPos[table_id])
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			memcpy(&tempNode->frame, src, PAGESIZE);
			bufferlast[table_id]->next = tempNode;
			tempNode->prev = bufferlast[table_id];
			bufferlast[table_id] = tempNode;
			bufferlast[table_id]->next = NULL;
			curPos[table_id]++;
		}
		else
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode,offset, table_id);
			memcpy(&tempNode->frame, src, PAGESIZE);
			file_write_page(get_page_offset(buffer[table_id]),&buffer[table_id]->frame, table_id);
			oldNode = buffer[table_id];	
			buffer[table_id] = buffer[table_id]->next;
			buffer[table_id]->prev = NULL;
			free(oldNode);
			bufferlast[table_id]->next = tempNode;
			tempNode->prev = bufferlast[table_id];
			bufferlast[table_id] = tempNode;
			bufferlast[table_id]->next = NULL;
		}
	}
}
