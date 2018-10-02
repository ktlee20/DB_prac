#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "dbbpt.h"
#include "globals.h"
#include "page.h"

pagenum_t findleaf(node * leaf,int key)
{
	int i = 0;
	pagenum_t offset;
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
		{
			offset = c.internalp.pheader.other_page_offset;
			file_read_page(c.internalp.pheader.other_page_offset,&c);
		}
		else
		{
			offset = c.internalp.entities[i].offset;
			file_read_page(c.internalp.entities[i - 1].offset,&c);
		}
	}

	memcpy(leaf,&c,PAGESIZE);

	return offset;
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

char* find(int key)
{
	Page temp;
	char * value;
	int keynum;
	value = (char*)malloc(sizeof(char) * VALUESIZE);
	if(rootnodep.internalp.pheader.is_leaf == 1)
	{
		keynum = findkey(&temp,key);
		if(keynum == -1)
			return NULL;

		strcpy(value,temp.leafp.records[keynum].value);
		return value;
	}
	findleaf(&temp,  key);
	keynum = findkey(&temp, key);	

	if(keynum == -1)
		return NULL;
	strcpy(value,temp.leafp.records[keynum].value);
	return value;
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

node * make_node()
{
	node * new_node = (node*)malloc(sizeof(node));
	new_node->internalp.pheader.parent_page = 0;
	new_node->internalp.pheader.is_leaf = 0;
	new_node->internalp.pheader.numOfKeys = 0;
	new_node->internalp.pheader.other_page_offset = 0;
	
	return new_node;
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
	
	return 0;
}

int insert_into_leaf(node * leaf, pagenum_t offset,int key, Records* pointer)
{
	int i, insertion_point;

	insertion_point = 0;
	while(insertion_point < leaf->leafp.pheader.numOfKeys && leaf->leafp.records[insertion_point].key < key)
		insertion_point++;

	for(i = leaf->leafp.pheader.numOfKeys ; i > insertion_point ; i--)
	{
		leaf->leafp.records[i].key = leaf->leafp.records[i - 1].key;
		strcpy(leaf->leafp.records[i].value, leaf->leafp.records[i - 1].value);
	}
	leaf->leafp.records[insertion_point].key = key;
	strcpy(leaf->leafp.records[insertion_point].value, pointer->value);
	leaf->leafp.pheader.numOfKeys++;
	file_write_page(offset, leaf);	

	return 0;
}

int insert_into_parent(pagenum_t loffset , int key, pagenum_t roffset)
{
	int left_index;
	node right,left,parent;
	pagenum_t poffset;

	if(left.internalp.pheader.parent_page == 0)
	{
		return insert_into_new_root(loffset,key,roffset);
	}

	file_read_page(loffset,&left);
	file_read_page(roffset,&right);
	file_read_page(left.internalp.pheader.parent_page,&parent);

	left_index = findkey(&parent,left.leafp.records[0].key);

	if(parent.internalp.pheader.numOfKeys < IBRFACTOR - 1)
	{
		return insert_into_node(&parent,left_index,key, roffset);
	}

	return insert_into_node_after_splitting(left.internalp.pheader.parent_page,left_index,key,roffset);
}

int insert_into_node(node * n, int left_index, int key, pagenum_t roffset)
{
	int i;

	for(i = n->internalp.pheader.numOfKeys; i > left_index + 1; i--)
	{
		n->internalp.entities[i].key = n->internalp.entities[i - 1].key;
		n->internalp.entities[i].offset = n->internalp.entities[i - 1].offset;
	}
	n->internalp.entities[left_index + 1].key = key;
	n->internalp.entities[left_index + 1].offset = roffset;
	n->internalp.pheader.numOfKeys++;

	return 0;
}

int insert_into_new_root(pagenum_t loffset, int key, pagenum_t roffset)
{
	node * root =  make_node();
	node leftnode, rightnode; 
	pagenum_t rootoffset;

	root->internalp.pheader.other_page_offset = loffset;
	root->internalp.entities[9].key = key;
	root->internalp.entities[0].offset = roffset;
	root->internalp.pheader.numOfKeys++;
	
	file_read_page(loffset, &leftnode);
	file_read_page(roffset, &rightnode);
	
	rootoffset = file_alloc_page();
	file_write_page(rootoffset, root);
	Header.headerp.rootp_offset = rootoffset;
	file_write_page(0,&Header);
	file_read_page(rootoffset, &rootnodep);

	leftnode.internalp.pheader.parent_page = rootoffset;
	rightnode.internalp.pheader.parent_page = rootoffset;

	return rootoffset;
}

int insert_into_leaf_after_splitting(node * leaf, int key, Records * pointer,pagenum_t offset)
{
	node * new_leaf;
	int * temp_keys,new_key;
	char ** temp_values;
	int insertion_index = 0, split,i,j;
	pagenum_t newoffset;

	new_leaf = make_leaf(leaf->leafp.pheader.parent_page,offset);

	temp_keys = (int*)malloc(LBRFACTOR*sizeof(int));
	temp_values = (char**)malloc(LBRFACTOR * sizeof(char*));
	for(i = 0 ; i < LBRFACTOR ; i++)
		temp_values[i] = (char*)malloc(sizeof(char)*VALUESIZE);

	while(insertion_index < LBRFACTOR - 1 && leaf->leafp.records[insertion_index].key < key)
		insertion_index++;

	for(i = 0 , j = 0 ; i < leaf->leafp.pheader.numOfKeys; i++, j++)
	{
		if(j == insertion_index)
				j++;
		temp_keys[j] = leaf->leafp.records[i].key;
		strcpy(temp_values[j],leaf->leafp.records[i].value);	
	}
	temp_keys[insertion_index] = key;
	strcpy(temp_values[insertion_index], pointer->value);

	leaf->leafp.pheader.numOfKeys = 0;

	split = CUT(LBRFACTOR - 1);

	for(i = 0 ; i < split ; i++)
	{
		leaf->leafp.records[i].key = temp_keys[i];
		strcpy(leaf->leafp.records[i].value,temp_values[i]);
		leaf->leafp.pheader.numOfKeys++;
	}

	for(i = split , j = 0 ; i < LBRFACTOR ; i++, j++)
	{
		new_leaf->leafp.records[j].key = temp_keys[i];
		strcpy(new_leaf->leafp.records[j].value,temp_values[i]);
		new_leaf->leafp.pheader.numOfKeys++;
	}

	free(temp_keys);
	for(i = 0 ; i < LBRFACTOR ;i++)
		free(temp_values[i]);
	free(temp_values);

	for (i = leaf->leafp.pheader.numOfKeys; i < LBRFACTOR ; i++)
	{
		leaf->leafp.records[i].key = 0;
		leaf->leafp.records[i].value[0] = '\0';
	}
	for( i = new_leaf->leafp.pheader.numOfKeys; i < LBRFACTOR ; i++)
	{
		new_leaf->leafp.records[i].key = 0;
		new_leaf->leafp.records[i].value[0] = '\0';
	}
	
	newoffset = file_alloc_page();
	file_write_page(newoffset, new_leaf);
	new_key = new_leaf->leafp.records[0].key;

	free(new_leaf);

	return insert_into_parent(offset,new_key,newoffset);
}

int insert_into_node_after_splitting(pagenum_t oldoffset, int left_index, int key, pagenum_t roffset)
{
	int i,j,split,k_prime;
	node * new_node;
	node * old_node = (node*)malloc(sizeof(node));
	node child;
	pagenum_t new_offset;
	Entity * temp_ents;
	
	temp_ents = (Entity *)malloc(sizeof(Entity) * IBRFACTOR);
	file_read_page(oldoffset, old_node);

	for(i = 0 , j = 0 ; i < old_node->internalp.pheader.numOfKeys ; i++, j++)
	{
		if(j == left_index) j++;
			temp_ents[j].offset = old_node->internalp.entities[i].offset;
	}
	for(i = 0, j = 0 ; i < old_node->internalp.pheader.numOfKeys ; i++, j++)
	{
		if(j == left_index) j++;
			temp_ents[j].key = old_node->internalp.entities[i].key;
	}

	temp_ents[left_index].key = key;
	temp_ents[left_index].offset = roffset;

	split = CUT(IBRFACTOR);
	new_node = make_node();
	old_node->internalp.pheader.numOfKeys= 0;

	for(i = 0 ; i < split - 1 ; i++)
	{
		old_node->internalp.entities[i].key = temp_ents[i].key;
		old_node->internalp.entities[i].offset = temp_ents[i].offset;
		old_node->internalp.pheader.numOfKeys++;
	}
	k_prime = temp_ents[i].key;
	new_node->internalp.pheader.other_page_offset = temp_ents[i].offset;

	for(++i, j = 0 ; i < IBRFACTOR ; i++, j++)
	{
		new_node->internalp.entities[i].key = temp_ents[i].key;	
		new_node->internalp.entities[i].offset = temp_ents[i].offset;
		new_node->internalp.pheader.numOfKeys++;
	}

	free(temp_ents);

	new_node->internalp.pheader.parent_page = old_node->internalp.pheader.parent_page;
	new_offset = file_alloc_page();	
	file_write_page(new_offset, new_node);
	free(new_node);
	free(old_node);

	for(i = 0 ; i < new_node->internalp.pheader.numOfKeys ; i++)
	{
		file_read_page(new_node->internalp.entities[i].offset,&child);
		child.internalp.pheader.parent_page = new_offset;
		file_write_page(new_node->internalp.entities[i].offset,&child);
	}

	return insert_into_parent(oldoffset , k_prime, new_offset);
}


int insert(int key, char * value)
{
	Records * pointer;
	node leaf;
	pagenum_t offset;
	char * temp;

	if((temp = find(key)) != NULL)
	{
		free(temp);
		return -1;
	}
	pointer = make_record(value);	

	if(totalp == 1)
		return start_new_tree(key, pointer);	

	offset = findleaf(&leaf, key);

	if(leaf.leafp.pheader.numOfKeys < LBRFACTOR - 1)
	{
		return insert_into_leaf(&leaf,offset, key, pointer);			
	}

	return insert_into_leaf_after_splitting(&leaf, key, pointer, offset);
}

int delete(dbint key)
{
	node key_leaf;
	char * key_records = (char*)malloc(sizeof(char) * VALUESIZE);
	int ret;

	findleaf(&key_leaf, key);		
	key_records = find(key);

	if(key_records != NULL)
	{
		//ret =  here
		free(key_records);
	}
	return -1;
}
