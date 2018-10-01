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
	memset(leaf,&c,PAGESIZE);
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

int find(int key)
{
	Page temp;
	int numOfKeys;
	if(rootnodep.internalp.pheader.is_leaf == 1)
		return findkey(&rootnodep,key);

	findleaf(&temp,  key);
	
}


Records* make_record(char * value)
{
	Records * retPointer = (Records*)malloc(sizeof(Records));
	strcpy(retPointer->value, value);

	return retPointer;
}

node * insert(int key, char * value)
{
	Records * pointer;
	node * leaf = &rootnodep;

	if(find(key) != -1)
		return NULL;

	pointer = make_record(value);	

	{
		
	}
}
