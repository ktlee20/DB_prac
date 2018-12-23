#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "defines.h"
#include "BufferManager.h"
#include "DiskManager.h"
#include "globals.h"


Buffer_Control_Block bufctrl[MAXTABLE + 1];
int numOfCol[MAXTABLE + 1];

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
	bufctrl[table_id].bufsize = num_buf;

	bufctrl[table_id].buffer = NULL;
	bufctrl[table_id].bufferlast = NULL;
	bufctrl[table_id].curPos = 0;
}

void closeBuf(int table_id)
{
	Bufstrt * deleteBuf = bufctrl[table_id].buffer;
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
	bufctrl[table_id].buffer = bufctrl[table_id].bufferlast = NULL;
	bufctrl[table_id].curPos = 0;
}


pagenum_t get_page_offset(Bufstrt * node)
{
	return node->page_num;
}

Bufstrt * find_leaf_buffer(pagenum_t offset, int table_id)
{
	Bufstrt * temp = bufctrl[table_id].buffer;
	
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
		if(bufctrl[table_id].curPos == 1)
		{
			bufctrl[table_id].buffer = bufctrl[table_id].bufferlast = NULL;
			if(offset != header[table_id]->headerp.rootp_offset)
				file_free_page(offset, table_id);
			bufctrl[table_id].curPos = 0;
			free(tempNode);
		}
		else if(bufctrl[table_id].curPos == 2)
		{
			if(bufctrl[table_id].buffer == tempNode)
			{
				bufctrl[table_id].buffer = bufctrl[table_id].bufferlast;
				bufctrl[table_id].buffer->prev = bufctrl[table_id].buffer->next = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}
			else
			{
				bufctrl[table_id].bufferlast = bufctrl[table_id].buffer;
				bufctrl[table_id].buffer->prev = bufctrl[table_id].buffer->next = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}			
			bufctrl[table_id].curPos--;
		}
		else
		{
			if(bufctrl[table_id].buffer == tempNode)
			{
				bufctrl[table_id].buffer = bufctrl[table_id].buffer->next;
				bufctrl[table_id].buffer->prev = NULL;
				if(offset != header[table_id]->headerp.rootp_offset)
					file_free_page(offset, table_id);
				free(tempNode);
			}
			else if(bufctrl[table_id].bufferlast == tempNode)
			{
				bufctrl[table_id].bufferlast = bufctrl[table_id].bufferlast->prev;
				bufctrl[table_id].bufferlast->next = NULL;
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
			bufctrl[table_id].curPos--;
		}
	}
}

Bufstrt * get_buf_node(pagenum_t offset, int table_id)
{
	Bufstrt * tempNode, * oldNode;
	pthread_mutex_lock(&bufctrl[table_id].buf_ctrl_mutex);
	
	tempNode = find_leaf_buffer(offset, table_id);	

	if(tempNode != NULL)
	{
		buf_lru_set(tempNode,table_id);
	}
	else
	{
		tempNode = makeBufNode();
		if(bufctrl[table_id].curPos == 0)
		{
			bufferAttr(tempNode, offset, table_id);
			bufctrl[table_id].buffer = bufctrl[table_id].bufferlast = tempNode;
			file_read_page(offset, &bufctrl[table_id].buffer->frame, table_id);
			bufctrl[table_id].curPos++;
		}
		else if(bufctrl[table_id].bufsize > bufctrl[table_id].curPos)
		{
			bufferAttr(tempNode, offset, table_id);
			bufctrl[table_id].bufferlast->next = tempNode;
			tempNode->prev = bufctrl[table_id].bufferlast;
			bufctrl[table_id].bufferlast = tempNode;
			file_read_page(offset, &bufctrl[table_id].bufferlast->frame, table_id);
			bufctrl[table_id].bufferlast->next = NULL;
			bufctrl[table_id].curPos++;
		}
		else
		{
			bufferAttr(tempNode, offset ,table_id);
			file_write_page(get_page_offset(bufctrl[table_id].buffer), &bufctrl[table_id].buffer->frame, table_id);
			oldNode = bufctrl[table_id].buffer;
			bufctrl[table_id].buffer = bufctrl[table_id].buffer->next;
			bufctrl[table_id].buffer->prev = NULL;
			free(oldNode);
			bufctrl[table_id].bufferlast->next = tempNode;
			tempNode->prev = bufctrl[table_id].bufferlast;
			bufctrl[table_id].bufferlast = tempNode;
			file_read_page(offset, &bufctrl[table_id].bufferlast->frame, table_id);
			bufctrl[table_id].bufferlast->next = NULL;
		}
	}
	
	return tempNode;
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
		if(bufctrl[table_id].curPos== 0)
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			bufctrl[table_id].buffer = bufctrl[table_id].bufferlast = tempNode;
			file_read_page(offset, &bufctrl[table_id].buffer->frame, table_id);
			bufctrl[table_id].curPos++;	
		}
		else if(bufctrl[table_id].bufsize > bufctrl[table_id].curPos)
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			bufctrl[table_id].bufferlast->next = tempNode;
			tempNode->prev = bufctrl[table_id].bufferlast;
			bufctrl[table_id].bufferlast = tempNode;
			file_read_page(offset, &bufctrl[table_id].bufferlast->frame, table_id);	
			bufctrl[table_id].bufferlast->next = NULL;
			bufctrl[table_id].curPos++;	
		}
		else
		{
			tempNode = makeBufNode();		
			bufferAttr(tempNode, offset, table_id);
			file_write_page(get_page_offset(bufctrl[table_id].buffer), &bufctrl[table_id].buffer->frame, table_id);
			oldNode = bufctrl[table_id].buffer;
			bufctrl[table_id].buffer = bufctrl[table_id].buffer->next;
			bufctrl[table_id].buffer->prev = NULL;
			free(oldNode);
			bufctrl[table_id].bufferlast->next = tempNode;
			tempNode->prev = bufctrl[table_id].bufferlast;
			bufctrl[table_id].bufferlast = tempNode;
			file_read_page(offset, &bufctrl[table_id].bufferlast->frame, table_id);
			bufctrl[table_id].bufferlast->next = NULL;
		}
	}
	
	memcpy(dest, &bufctrl[table_id].bufferlast->frame, PAGESIZE);
	return &bufctrl[table_id].bufferlast->frame;
}

void buf_lru_set(Bufstrt * node, int table_id)
{
	Bufstrt * tempNode = node;
	
	if(tempNode->next == NULL)
		return;

	if(tempNode->prev == NULL)
	{
		tempNode->next->prev = NULL;
		bufctrl[table_id].buffer = tempNode->next;
		bufctrl[table_id].bufferlast->next = tempNode;
		tempNode->prev = bufctrl[table_id].bufferlast;
		tempNode->next = NULL;
		bufctrl[table_id].bufferlast = tempNode;

		return;
	}
	
	tempNode->next->prev = tempNode->prev;
	tempNode->prev->next = tempNode->next;
	
	bufctrl[table_id].bufferlast->next = tempNode;
	tempNode->prev = bufctrl[table_id].bufferlast;
	tempNode->next = NULL;
	bufctrl[table_id].bufferlast = tempNode;

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
		if(bufctrl[table_id].curPos == 0)
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			memcpy(&tempNode->frame, src, PAGESIZE);
			bufctrl[table_id].buffer = bufctrl[table_id].bufferlast = tempNode;
			bufctrl[table_id].curPos++;	
		}
		else if(bufctrl[table_id].bufsize > bufctrl[table_id].curPos)
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode, offset, table_id);
			memcpy(&tempNode->frame, src, PAGESIZE);
			bufctrl[table_id].bufferlast->next = tempNode;
			tempNode->prev = bufctrl[table_id].bufferlast;
			bufctrl[table_id].bufferlast = tempNode;
			bufctrl[table_id].bufferlast->next = NULL;
			bufctrl[table_id].curPos++;
		}
		else
		{
			tempNode = makeBufNode();
			bufferAttr(tempNode,offset, table_id);
			memcpy(&tempNode->frame, src, PAGESIZE);
			file_write_page(get_page_offset(bufctrl[table_id].buffer),&bufctrl[table_id].buffer->frame, table_id);
			oldNode = bufctrl[table_id].buffer;	
			bufctrl[table_id].buffer= bufctrl[table_id].buffer->next;
			bufctrl[table_id].buffer->prev = NULL;
			free(oldNode);
			bufctrl[table_id].bufferlast->next = tempNode;
			tempNode->prev = bufctrl[table_id].bufferlast;
			bufctrl[table_id].bufferlast = tempNode;
			bufctrl[table_id].bufferlast->next = NULL;
		}
	}

	if(offset != 0 &&( offset == header[table_id]->headerp.rootp_offset / PAGESIZE))
		buf_read_page(offset, rootpage[table_id],table_id);
}
