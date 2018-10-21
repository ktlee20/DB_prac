#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbbpt.h"
#include "defines.h"
#include "globals.h"
#include "DiskManager.h"

node * make_node(pagenum_t parent_off, pagenum_t otherp_off)
{
	node * new_node;
	new_node = (node*)malloc(sizeof(node));
	new_node->internalp.pheader.is_leaf = FALSE;
	new_node->internalp.pheader.numOfKeys = 0;
	new_node->internalp.pheader.parent_page = parent_off;
	new_node->internalp.pheader.other_page_offset = otherp_off;

	return new_node;
}
node * make_leaf(pagenum_t parent_off, pagenum_t otherp_off)
{
	node * leaf = make_node(parent_off, otherp_off);
	leaf->leafp.pheader.is_leaf = TRUE;

	return leaf;
}

node * start_new_tree(dbint key, Records* pointer)
{
	pagenum_t rootnum = file_alloc_page();
	rootpage = make_leaf(0,0);
	rootpage->leafp.records[0].key = key;
	strcpy(rootpage->leafp.records[0].value,pointer->value);
	rootpage->leafp.pheader.numOfKeys++;
	file_write_page(rootnum, rootpage);	
	chgheadroff(rootnum);

	return rootpage;
}

node * insert_into_leaf(node * leaf, dbint key, Records* pointer,pagenum_t offset)
{
	int i, insertion_point;

	insertion_point = 0;
	while(insertion_point < leaf->leafp.pheader.numOfKeys && leaf->leafp.records[insertion_point].key <key)
		insertion_point++;

	for(i = leaf->leafp.pheader.numOfKeys; i > insertion_point ; i--)
	{
		leaf->leafp.records[i].key = leaf->leafp.records[i - 1].key;
		strcpy(leaf->leafp.records[i].value, leaf->leafp.records[i - 1].value);
	}
	leaf->leafp.records[insertion_point].key = key;
	strcpy(leaf->leafp.records[insertion_point].value,pointer->value);
	leaf->leafp.pheader.numOfKeys++;
	
	file_write_page(offset, leaf);
	return leaf;

}

node * insert_into_leaf_after_splitting(node * leaf, dbint key, Records* pointer, pagenum_t old_offset)
{
	node * new_leaf, *forfree;
	Records* temp_records;
	int insertion_index = 0, split, i, j,k;
	dbint new_key;
	pagenum_t new_offset;

	new_leaf = make_leaf(leaf->leafp.pheader.parent_page,0);

	temp_records = (Records*)malloc(sizeof(Records) * LBRFACTOR );
	while(insertion_index < LBRFACTOR - 1 && leaf->leafp.records[insertion_index].key < key)
		insertion_index++;

	for(i = 0 , j = 0 ; i < leaf->leafp.pheader.numOfKeys ;i++,j++)
	{
		if(j == insertion_index)
			j++;
		temp_records[j].key = leaf->leafp.records[i].key;
		strcpy(temp_records[j].value, leaf->leafp.records[i].value);
	}

	temp_records[insertion_index].key = key;
	strcpy(temp_records[insertion_index].value, pointer->value);

	leaf->leafp.pheader.numOfKeys = 0;
	split = CUT(LBRFACTOR);

	for(i = 0 ; i < split ; i++)
	{
		leaf->leafp.records[i].key = temp_records[i].key;
		strcpy(leaf->leafp.records[i].value, temp_records[i].value);
		leaf->leafp.pheader.numOfKeys++;
	}

	/*
	for(k = leaf->leafp.pheader.numOfKeys ; k < LBRFACTOR ; k++)
	{
		leaf->leafp.records[k].key = 0;
		leaf->leafp.records[k].value[0] = '\0';
	}
	*/
	for(i = split ,j = 0 ; i < LBRFACTOR ; i++ , j++)
	{
		new_leaf->leafp.records[j].key = temp_records[i].key;
		strcpy(new_leaf->leafp.records[j].value, temp_records[i].value);
		new_leaf->leafp.pheader.numOfKeys++;
	}

	/*
	for(k = new_leaf->leafp.pheader.numOfKeys ; k < LBRFACTOR ; k++)
	{
		new_leaf->leafp.records[k].key = 0;
		new_leaf->leafp.records[k].value[0] = '\0';
	}
	*/
	free(temp_records);

	new_leaf->leafp.pheader.parent_page = leaf->leafp.pheader.parent_page;
	new_offset = file_alloc_page();
	new_key = new_leaf->leafp.records[0].key;
	leaf->leafp.pheader.other_page_offset = new_offset;	

	file_write_page(new_offset, new_leaf);
	file_write_page(old_offset, leaf);

	forfree = insert_into_parent(leaf,old_offset,new_key, new_leaf, new_offset);
	free(new_leaf);

	return forfree;
}

int get_left_index(node * parent, pagenum_t loffset)
{
	int left_index = 0;
	if(parent->internalp.pheader.other_page_offset == loffset)
		return -1;
	while(left_index < parent->internalp.pheader.numOfKeys && parent->internalp.entities[left_index].offset != loffset)
	left_index++;

	return left_index;
}

node * insert_into_parent(node * left, pagenum_t loffset, dbint key, node * right, pagenum_t roffset)
{
	int left_index;
	node * parent = (node*)malloc(sizeof(node));
	node * temp;
	pagenum_t parent_offset = left->leafp.pheader.parent_page;


	if(parent_offset == 0)
		return insert_into_new_root(left,loffset,key,right,roffset);

	file_read_page(parent_offset,parent);

	left_index = get_left_index(parent, loffset);

	if(parent->internalp.pheader.numOfKeys < IBRFACTOR-1)
	{
		temp = insert_into_node(parent,left_index,key,right, roffset);
		free(parent);
		return temp;
	}
	temp = insert_into_node_after_splitting(parent, left->leafp.pheader.parent_page,left_index,key,right,roffset);
	free(parent);
	return temp;
}

node * insert_into_node(node * n, int left_index, dbint key, node * right, pagenum_t roffset)
{
	int i;

	for(i = n->internalp.pheader.numOfKeys - 1; i > left_index ; i--)
	{
		n->internalp.entities[i + 1].offset = n->internalp.entities[i].offset;
		n->internalp.entities[i + 1].key = n->internalp.entities[i].key;
	}
	n->internalp.entities[left_index + 1].offset = roffset;
	n->internalp.entities[left_index + 1].key = key;
	n->internalp.pheader.numOfKeys++;

	file_write_page(right->internalp.pheader.parent_page,n);	

	return rootpage;
}

node * insert_into_node_after_splitting(node * old_node, pagenum_t old_offset, int left_index, dbint key, node * right, pagenum_t roffset)
{
	int i,j,k,split;
	dbint k_prime;
	node * new_node, * child;
	pagenum_t zero_offset = old_node->internalp.pheader.other_page_offset;
	node temp,*forfree,temp2;
	pagenum_t new_offset;
	dbint * temp_keys;
	pagenum_t * temp_offsets;

	temp_keys = (dbint*)malloc(sizeof(dbint)*IBRFACTOR);
	temp_offsets = (pagenum_t*)malloc(sizeof(pagenum_t)*(IBRFACTOR + 1));

	temp_offsets[0] = zero_offset;
	for(i = 0 , j = 1 ; i < old_node->internalp.pheader.numOfKeys ;i++ , j++)
	{
		if(j == left_index + 2)
			j++;
		temp_offsets[j] = old_node->internalp.entities[i].offset;
	}

	for(i = 0 , j = 0 ; i < old_node->internalp.pheader.numOfKeys ; i++, j++)
	{
		if(j == left_index + 1)
			j++;
		temp_keys[j] = old_node->internalp.entities[i].key;
	}
	temp_offsets[left_index + 2] = roffset;
	temp_keys[left_index + 1] = key;

	split = CUT(IBRFACTOR);
	new_node = make_node(old_node->internalp.pheader.parent_page,0);
	old_node->internalp.pheader.numOfKeys = 0;
	for(i = 0 ; i < split ; i++)
	{
		 old_node->internalp.entities[i].key = temp_keys[i];
		 old_node->internalp.entities[i].offset = temp_offsets[i + 1];
		 old_node->internalp.pheader.numOfKeys++;
	}
	/*
	for(k = old_node->internalp.pheader.numOfKeys ; k < IBRFACTOR - 1; i++)
	{
		old_node->internalp.entities[k].key = 0;
		old_node->internalp.entities[k].offset = 0;
	}*/
	k_prime = temp_keys[i];
	new_node->internalp.pheader.other_page_offset = temp_offsets[i + 1];
	for(++i, j = 0; i < IBRFACTOR ; i++,j++)
	{
		 new_node->internalp.entities[j].key = temp_keys[i];
		 new_node->internalp.entities[j].offset = temp_offsets[i + 1];
		 new_node->internalp.pheader.numOfKeys++;
	}
	/*
	for(k = new_node->internalp.pheader.numOfKeys ; k < IBRFACTOR - 1 ; k++)
	{
		new_node->internalp.entities[k].key = 0;
		new_node->internalp.entities[k].offset = 0;
	}*/

	free(temp_keys);
	free(temp_offsets);
	new_node->internalp.pheader.parent_page = old_node->internalp.pheader.parent_page;
	new_offset = file_alloc_page();
	file_write_page(new_offset, new_node);
	file_write_page(old_offset, old_node);

	file_read_page(new_node->internalp.pheader.other_page_offset, &temp);
	temp.internalp.pheader.parent_page = new_offset;
	file_write_page(new_node->internalp.pheader.other_page_offset, &temp);

	file_read_page(old_node->internalp.pheader.other_page_offset, &temp2);
	file_read_page(old_node->internalp.entities[1].offset, &temp2);

	for(i = 0 ; i < new_node->internalp.pheader.numOfKeys; i++)
	{
		file_read_page(new_node->internalp.entities[i].offset,&temp);
		temp.internalp.pheader.parent_page = new_offset;
		file_write_page(new_node->internalp.entities[i].offset,&temp);
	}

	forfree = insert_into_parent(old_node, old_offset, k_prime, new_node, new_offset);

	free(new_node);
	return forfree;
}

node * insert_into_new_root(node * left, pagenum_t loffset, dbint key, node * right ,pagenum_t roffset)
{
	node * root = make_node(0,0);
	pagenum_t newROffset;

	root->internalp.entities[0].key = key;
	root->internalp.pheader.other_page_offset = loffset;
	root->internalp.entities[0].offset = roffset;
	root->internalp.pheader.numOfKeys++;
	
	newROffset = file_alloc_page();
	left->internalp.pheader.parent_page = newROffset;
	right->internalp.pheader.parent_page = newROffset;
	chgheadroff(newROffset);
	file_write_page(newROffset, root);
	file_write_page(loffset , left);
	file_write_page(roffset, right);

	free(root);
	return rootpage;
}

node * dbinsert(node * root, dbint key, char * value)
{
	Records* pointer, *forfree;
	node * leaf;
	pagenum_t offset;

	if((forfree = dbfind(root, key,&offset)) != NULL)
	{
		free(forfree);
		return NULL;
	}
	pointer = make_records(value);

	if(root == NULL)
		return start_new_tree(key, pointer);

	leaf = find_leaf(root, key,&offset);

	if(leaf->leafp.pheader.numOfKeys < LBRFACTOR - 1)
	{
		leaf = insert_into_leaf(leaf, key, pointer,offset);
		free(leaf);
		free(pointer);
		return root;
	}

	insert_into_leaf_after_splitting(leaf,key,pointer,offset);	
	free(leaf);
	free(pointer);
	return root;
}

Records* make_records(char * value)
{
	Records* new_record = (Records*)malloc(sizeof(Records));
	strcpy(new_record->value, value);	

	return new_record;
}

Records * dbfind(node * root, dbint key, pagenum_t * poffset)
{
	int i = 0;
	node * c = find_leaf(root, key, poffset);
	Records* ret = (Records*)malloc(sizeof(Records));;
	if(c == NULL)
	{
		free(ret);
		return NULL;
	}
	
	for(i = 0 ; i < c->leafp.pheader.numOfKeys ; i ++)
		if(c->leafp.records[i].key == key)
			break;
	
	if(i == c->leafp.pheader.numOfKeys)
	{
		free(ret);
		free(c);
		return NULL;
	}
	else
	{
		memcpy(ret,&c->leafp.records[i],sizeof(Records));
		free(c);
		return ret;
	}
}

node * find_leaf(node * root, dbint key, pagenum_t * poffset)
{
	int i = 0;
	node * c = root; 
	node * ret = (node*)malloc(sizeof(node));
	pagenum_t offset = header->headerp.rootp_offset;


	if(c == NULL)
		return c;

	c = (node*)malloc(sizeof(node));

	file_read_page(offset,c);

	while(!c->internalp.pheader.is_leaf)
	{
		i = 0;
		while(i < c->internalp.pheader.numOfKeys)
		{
			if(key >= c->internalp.entities[i].key)
				i++;
			else
				break;
		}
		if(i == 0)
		{
			offset = c->internalp.pheader.other_page_offset;
			file_read_page(c->internalp.pheader.other_page_offset,c);
		}
		else
		{
			offset = c->internalp.entities[i - 1].offset;
			file_read_page(c->internalp.entities[i - 1].offset,c);
		}
	}

	memcpy(ret,c,PAGESIZE);

	if(c->internalp.pheader.parent_page != 0)
		*poffset = offset;
	else
		*poffset = header->headerp.rootp_offset;

	free(c);
	return ret;
}

node* dbdelete(node * root, dbint key)
{
	node * key_leaf;
	Records* key_record;
	pagenum_t poffset;

	key_record = dbfind(rootpage, key, &poffset);
	key_leaf = find_leaf(rootpage, key, &poffset);

	if(key_record != NULL && key_leaf != NULL)
	{
		root=delete_entry(key_leaf,key,key_record ,poffset);
		free(key_record);
		free(key_leaf);

		return root;
	}
	return NULL;
}

node * delete_entry(node * n, dbint key, void * pointer,pagenum_t offset)
{
	node * neighbor,* temp;
	int neighbor_index;
	pagenum_t neighbor_offset;

	n = remove_entry_from_node(n,key,pointer,offset);

	if(n->internalp.pheader.parent_page == 0)
		return adjust_root(n);

	//using delayed merge
	if(n->internalp.pheader.numOfKeys != 0)
		return rootpage;
	
	neighbor_index = get_neighbor_index(n, offset,&neighbor_offset);
	temp = coalesce_nodes(n ,offset,neighbor_offset ,neighbor_index);

	return rootpage;
}

int get_neighbor_index(node * n,pagenum_t offset, pagenum_t * neighbor_offset)
{
	int i;
	node temp;
	
	file_read_page(n->internalp.pheader.parent_page,&temp);
	
	if(temp.internalp.pheader.other_page_offset == offset)
	{
		*neighbor_offset = -1;
		return -2;
	}

	for(i = 0 ; i < temp.internalp.pheader.numOfKeys ; i++)
		if(temp.internalp.entities[i].offset == offset)
			break;	

		if(i == 0)
			*neighbor_offset = temp.internalp.pheader.other_page_offset;
		else
			*neighbor_offset = temp.internalp.entities[i - 1].offset;

	return i - 1;
}

node * adjust_root(node * n)
{
	node * root = rootpage;
	pagenum_t newRoffset ,oldRoffset;

	if(root->leafp.pheader.numOfKeys > 1)
		return root;

	if(!root->leafp.pheader.is_leaf)
	{
		if((root->internalp.pheader.other_page_offset == 0) && (root->internalp.entities[0].offset == 0))
		{
			file_free_page(header->headerp.rootp_offset);
			free(rootpage);
			rootpage = NULL;
			chgheadroff(0);

			return NULL;
		}			

		if((root->internalp.pheader.other_page_offset == 0) || (root->internalp.entities[0].offset == 0))
		{
			newRoffset = (root->internalp.pheader.other_page_offset == 0) ? root->internalp.entities[0].offset : root->internalp.pheader.other_page_offset;
			oldRoffset = header->headerp.rootp_offset;
			chgheadroff(newRoffset);		
			file_free_page(oldRoffset);
			file_read_page(newRoffset, rootpage);

			return root;
		}		
	}
	else
	{
		file_free_page(header->headerp.rootp_offset);
		header->headerp.rootp_offset = 0;
		file_write_page(0,header);
		free(rootpage);
		rootpage= NULL;
	}

	return rootpage;
}

node * remove_entry_from_node(node * n, dbint key, void * pointer,pagenum_t offset)
{
	int i, num_pointers;

	i = 0;
	if(n->leafp.pheader.is_leaf)
	{
		while(n->leafp.records[i].key != key)
			i++;
		for(++i; i < n->leafp.pheader.numOfKeys; i++)
		{
			n->leafp.records[i - 1].key = n->leafp.records[i].key;
			strcpy(n->leafp.records[i - 1].value,n->leafp.records[i].value);
		}
		while(i < LBRFACTOR - 1)
		{
			n->leafp.records[i].key = 0;
			n->leafp.records[i].value[0] ='\0';
			i++;
		}
	}
	else
	{
		n->internalp.pheader.other_page_offset = 0;
		n->internalp.entities[0].offset = 0;

	}
	n->internalp.pheader.numOfKeys--;
	file_write_page(offset, n);
	return n;
}

node * coalesce_nodes(node * n, pagenum_t curoffset, pagenum_t noffset, int n_index)
{
	int i,j;
	pagenum_t temp_off,poffset;
	node *parent = (node*)malloc(sizeof(node));
	node *forfree;
	node temp;

	file_read_page(n->internalp.pheader.parent_page, parent);
	poffset = n->internalp.pheader.parent_page;

	file_free_page(curoffset);
	if(parent->internalp.pheader.numOfKeys > 1)
	{
		if(n_index == -2)
		{
			parent->internalp.pheader.numOfKeys--;
			parent->internalp.pheader.other_page_offset = parent->internalp.entities[0].offset;
			for(i = 0 ; i < parent->internalp.pheader.numOfKeys ; i++)
			{
				parent->internalp.entities[i].key = parent->internalp.entities[i + 1].key;
				parent->internalp.entities[i].offset = parent->internalp.entities[i + 1].offset;
			}
			parent->internalp.entities[i].key = 0;
			parent->internalp.entities[i].offset = 0 ;
		}
		else
		{
			parent->internalp.pheader.numOfKeys--;
			for(i = n_index + 1 ; i < parent->internalp.pheader.numOfKeys ; i++)
			{
				parent->internalp.entities[i].key = parent->internalp.entities[i + 1].key;
				parent->internalp.entities[i].offset= parent->internalp.entities[i + 1].offset;
			}
			parent->internalp.entities[i].key = 0;
			parent->internalp.entities[i].offset = 0 ;
		}

		file_write_page(poffset, parent);
		free(parent);
		return rootpage;
	}

	if(parent->internalp.pheader.numOfKeys == 1 && (parent->internalp.entities[0].offset == 0 || parent->internalp.pheader.other_page_offset == 0))
	{
		rootpage = delete_entry(parent,parent->internalp.entities[0].key,n,n->internalp.pheader.parent_page);
		free(parent);

		return rootpage; 
	}
	
	if(n_index == -2)
	{
		temp_off = parent->internalp.pheader.other_page_offset;
		parent->internalp.pheader.other_page_offset = 0;

		file_write_page(poffset, parent);
		free(parent);
		return rootpage;
	}
	else if(n_index == -1)
	{
		parent->internalp.entities[0].offset = 0;
		file_write_page(poffset, parent);
		free(parent);
		return rootpage;
	}
}
