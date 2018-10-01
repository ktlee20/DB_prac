#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "dbbpt.h"
#include "globals.h"
#include "page.h"

void findleaf(node * leaf,int key)
{
	int i = 0;
	node c; 

	file_read_page(Header.headerp.rootp_offset,&c);
	while(!c.internalp.pheader.is_leaf)
	{
		i = 0;
		while(i < c.internalp.pheader.numOfKeys)
		{
			if(key >= c.internalp.entities[i].key)
				i++;
			else 
				break;
		}
		if(i == 0)
			file_read_page(c.internalp.pheader.other_page_offset,&c);
		else
			file_read_page(c.internalp.entities[i - 1].offset,&c);
	}

	memcpy(leaf,&c,PAGESIZE);
}

int findkey(node * leaf, int key)
{
	int i;
	int numOfKey = leaf->leafp.pheader.numOfKeys;

	
	for(i = 0 ; i < numOfKey; i++)
	{
		if(leaf->leafp.records[i].key == key)
			break;
	}
	
	if(i == numOfKey)
		i = -1;

	return i;
}

char* find(node * leaf, int key)
{
	Page temp;
	int keynum;
	if(rootnodep.internalp.pheader.is_leaf == 1)
	{
		keynum = findkey(&temp,key);
		return (keynum == -1) ? NULL : temp.leafp.records[keynum].value;
	}
	findleaf(&temp,  key);
	keynum = findkey(&temp, key);	
	memcpy(leaf,&temp,PAGESIZE);

	return (keynum == -1) ? NULL : temp.leafp.records[keynum].value;
}


Records* make_record(char * value)
{
	Records * retPointer = (Records*)malloc(sizeof(Records));
	strcpy(retPointer->value, value);

	return retPointer;
}

node * make_leaf(dbint parent_page, dbint other_page_offset)
{
	node * leaf = (node*)malloc(sizeof(node));
	leaf->leafp.pheader.parent_page = parent_page;
	leaf->leafp.pheader.is_leaf = 1;
	leaf->leafp.pheader.numOfKeys = 0;
	leaf->leafp.pheader.other_page_offset = other_page_offset;

	return leaf;
}

int start_new_tree(int key, Records* pointer)
{
	pagenum_t pages;
	node * root = make_leaf(0,0);
	root->leafp.records[0].key = key;
	root->leafp.pheader.numOfKeys++;
	strcpy(root->leafp.records[0].value, pointer->value);

	pages = file_alloc_page();
	file_write_page(pages,root);
	Header.headerp.rootp_offset = pages;
	file_write_page(0,&Header);
	
	return pages;
}

int insert(int key, char * value)
{
	Records * pointer;
	node leaf;

	if(find(&leaf,key) != NULL)
		return -1;

	pointer = make_record(value);	

	if(totalp == 1)
		return start_new_tree(key, pointer);	
		
}
