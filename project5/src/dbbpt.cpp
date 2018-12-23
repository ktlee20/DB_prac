#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BufferManager.h"
#include "dbbpt.h"
#include "defines.h"
#include "globals.h"
#include "DiskManager.h"

Page ** header;
Page ** rootpage;
pagenum_t * totalp;
pagenum_t * currentp;

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

node * start_new_tree(dbint key, Records* pointer,int i)
{
	pagenum_t rootnum = file_alloc_page(i);
	rootpage[i] = make_leaf(0,0);
	rootpage[i]->leafp.records[0].key = key;
	Insert_value(rootpage[i]->leafp.records[0].values,i,pointer);
	rootpage[i]->leafp.pheader.numOfKeys++;
	buf_write_page(rootnum, rootpage[i],i);	
	chgheadroff(rootnum,i);
	free(pointer);

	return rootpage[i];
}

node * insert_into_leaf(node * leaf, dbint key, Records* pointer,pagenum_t offset, int j)
{
	int i, insertion_point;

	insertion_point = 0;
	while(insertion_point < leaf->leafp.pheader.numOfKeys && leaf->leafp.records[insertion_point].key <key)
		insertion_point++;

	for(i = leaf->leafp.pheader.numOfKeys; i > insertion_point ; i--)
	{
		leaf->leafp.records[i].key = leaf->leafp.records[i - 1].key;
		Insert_value(leaf->leafp.records[i].values,j, &leaf->leafp.records[i-1]);
	}
	leaf->leafp.records[insertion_point].key = key;
	Insert_value(leaf->leafp.records[insertion_point].values,j,pointer);
	leaf->leafp.pheader.numOfKeys++;
	
	buf_write_page(offset/PAGESIZE, leaf,j);

	return leaf;
}

node * insert_into_leaf_after_splitting(node * leaf, dbint key, Records* pointer, pagenum_t old_offset ,int p)
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
		Insert_value(temp_records[j].values, p, &leaf->leafp.records[i]);
	}

	temp_records[insertion_index].key = key;
	Insert_value(temp_records[insertion_index].values, p, pointer);

	leaf->leafp.pheader.numOfKeys = 0;
	split = CUT(LBRFACTOR);

	for(i = 0 ; i < split ; i++)
	{
		leaf->leafp.records[i].key = temp_records[i].key;
		Insert_value(leaf->leafp.records[i].values, p, &temp_records[i]);
		leaf->leafp.pheader.numOfKeys++;
	}

	for(i = split ,j = 0 ; i < LBRFACTOR ; i++ , j++)
	{
		new_leaf->leafp.records[j].key = temp_records[i].key;
		Insert_value(new_leaf->leafp.records[j].values, p, &temp_records[i]);
		new_leaf->leafp.pheader.numOfKeys++;
	}

	free(temp_records);

	new_leaf->leafp.pheader.parent_page = leaf->leafp.pheader.parent_page;
	new_offset = file_alloc_page(p);
	new_key = new_leaf->leafp.records[0].key;
	leaf->leafp.pheader.other_page_offset = new_offset * PAGESIZE;	

	buf_write_page(new_offset, new_leaf,p);
	buf_write_page(old_offset/PAGESIZE, leaf,p);

	forfree = insert_into_parent(leaf,old_offset,new_key, new_leaf, new_offset,p);
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

node * insert_into_parent(node * left, pagenum_t loffset, dbint key, node * right, pagenum_t roffset,int i)
{
	int left_index;
	node * parent = (node*)malloc(sizeof(node));
	node * temp;
	pagenum_t parent_offset = left->leafp.pheader.parent_page;


	if(parent_offset == 0)
		return insert_into_new_root(left,loffset,key,right,roffset,i);

	buf_read_page(parent_offset/PAGESIZE,parent,i);

	left_index = get_left_index(parent, loffset);

	if(parent->internalp.pheader.numOfKeys < IBRFACTOR-1)
	{
		temp = insert_into_node(parent,left_index,key,right, roffset,i);
		free(parent);
		return temp;
	}
	temp = insert_into_node_after_splitting(parent, left->leafp.pheader.parent_page,left_index,key,right,roffset,i);
	free(parent);
	return temp;
}

node * insert_into_node(node * n, int left_index, dbint key, node * right, pagenum_t roffset, int j)
{
	int i;

	for(i = n->internalp.pheader.numOfKeys - 1; i > left_index ; i--)
	{
		n->internalp.entities[i + 1].offset = n->internalp.entities[i].offset;
		n->internalp.entities[i + 1].key = n->internalp.entities[i].key;
	}
	n->internalp.entities[left_index + 1].offset = roffset * PAGESIZE;
	n->internalp.entities[left_index + 1].key = key;
	n->internalp.pheader.numOfKeys++;

	buf_write_page(right->internalp.pheader.parent_page/PAGESIZE,n,j);	
	
	return rootpage[j];
}

node * insert_into_node_after_splitting(node * old_node, pagenum_t old_offset, int left_index, dbint key, node * right, pagenum_t roffset, int p)
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
	temp_offsets[left_index + 2] = roffset * PAGESIZE;
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

	k_prime = temp_keys[i];
	new_node->internalp.pheader.other_page_offset = temp_offsets[i + 1];
	for(++i, j = 0; i < IBRFACTOR ; i++,j++)
	{
		 new_node->internalp.entities[j].key = temp_keys[i];
		 new_node->internalp.entities[j].offset = temp_offsets[i + 1];
		 new_node->internalp.pheader.numOfKeys++;
	}

	free(temp_keys);
	free(temp_offsets);
	new_node->internalp.pheader.parent_page = old_node->internalp.pheader.parent_page;
	new_offset = file_alloc_page(p);

	buf_write_page(new_offset, new_node,p);
	buf_write_page((old_offset / PAGESIZE), old_node,p);

	buf_read_page(new_node->internalp.pheader.other_page_offset/PAGESIZE, &temp,p);
	temp.internalp.pheader.parent_page = new_offset * PAGESIZE;
	
	buf_write_page(new_node->internalp.pheader.other_page_offset/PAGESIZE, &temp, p);
	buf_read_page(old_node->internalp.pheader.other_page_offset/PAGESIZE, &temp2,p);

	buf_read_page(old_node->internalp.entities[1].offset/PAGESIZE, &temp2,p);

	for(i = 0 ; i < new_node->internalp.pheader.numOfKeys; i++)
	{
		buf_read_page(new_node->internalp.entities[i].offset/PAGESIZE,&temp,p);
		temp.internalp.pheader.parent_page = new_offset * PAGESIZE;
		buf_write_page(new_node->internalp.entities[i].offset/PAGESIZE,&temp,p);
	}

	forfree = insert_into_parent(old_node, old_offset, k_prime, new_node, new_offset,p);

	free(new_node);
	return forfree;
}

node * insert_into_new_root(node * left, pagenum_t loffset, dbint key, node * right ,pagenum_t roffset,int p)
{
	node * root = make_node(0,0);
	pagenum_t newROffset;

	root->internalp.entities[0].key = key;
	root->internalp.pheader.other_page_offset = loffset;
	root->internalp.entities[0].offset = roffset * PAGESIZE;
	root->internalp.pheader.numOfKeys++;
	
	newROffset = file_alloc_page(p);
	left->internalp.pheader.parent_page = newROffset * PAGESIZE;
	right->internalp.pheader.parent_page = newROffset * PAGESIZE;
	chgheadroff(newROffset,p);
	buf_write_page(newROffset, root,p);
	buf_write_page(loffset/PAGESIZE , left,p);
	buf_write_page(roffset, right,p);

	free(root);
	return rootpage[p];
}

node * dbinsert(node * root, dbint key, dbint* value,int p)
{
	Records* pointer, *forfree;
	node * leaf;
	pagenum_t offset;

	if((forfree = dbfind(root, key,&offset,p)) != NULL)
	{
		free(forfree);
		return NULL;
	}

	pointer = make_records(value, p);

	if(root == NULL)
		return start_new_tree(key, pointer,p);

	leaf = find_leaf(root, key,&offset,p);

	if(leaf->leafp.pheader.numOfKeys < LBRFACTOR - 1)
	{
		leaf = insert_into_leaf(leaf, key, pointer,offset,p);
		free(leaf);
		free(pointer);
		return root;
	}

	insert_into_leaf_after_splitting(leaf,key,pointer,offset,p);	
	free(leaf);
	free(pointer);
	return root;
}

void Insert_value(dbint * value, int tnum, Records * record)
{
	int i, j = numOfCol[tnum];

	for(i = 0 ; i < j - 1 ; i++)
		value[i] = record->values[i]; 	
}

Records* make_records(dbint* value,int tnum)
{
	Records* new_record = (Records*)malloc(sizeof(Records));
	Records* temp = (Records*)malloc(sizeof(Records));
	int i;
	for(i = 0 ; i < numOfCol[tnum] - 1 ; i++)
		temp->values[i] = value[i];

	Insert_value(new_record->values, tnum,temp );
	free(temp);

	return new_record;
}

Records * dbfind(node * root, dbint key, pagenum_t * poffset,int p)
{
	int i = 0;
	node * c = find_leaf(root, key, poffset,p);
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

node * find_leaf(node * root, dbint key, pagenum_t * poffset, int p)
{
	int i = 0;
	node * c = root; 
	node * ret = (node*)malloc(sizeof(node));
	pagenum_t offset = header[p]->headerp.rootp_offset;

	if(c == NULL)
	{
		free(ret);
		return c;
	}

	c = (node*)malloc(sizeof(node));

	buf_read_page(offset/PAGESIZE,c,p);
		
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
			buf_read_page(c->internalp.pheader.other_page_offset/PAGESIZE,c,p);
		}
		else
		{
			offset = c->internalp.entities[i - 1].offset;
			buf_read_page(c->internalp.entities[i - 1].offset/PAGESIZE,c,p);
		}
	}

	memcpy(ret,c,PAGESIZE);

	if(c->internalp.pheader.parent_page != 0)
		*poffset = offset;
	else
		*poffset = header[p]->headerp.rootp_offset;

	free(c);
	return ret;
}

dbint get_leaf_page_id(node * root, dbint key, int p)
{
	int i =0;
	node * c = root;
	pagenum_t offset = header[p]->headerp.rootp_offset;

	if(c == NULL)
	{
		return -1;
	}
	c = (node*)malloc(sizeof(node));
	file_read_page(offset/PAGESIZE , c , p);
	while(!(c->internalp.pheader.is_leaf))
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
			file_read_page(c->internalp.pheader.other_page_offset/PAGESIZE, c, p);
		}
		else
		{
			offset = c->internalp.entities[i - 1].offset;
			file_read_page(c->internalp.entities[i - 1].offset / PAGESIZE , c, p);
		}
	}

	for(i = 0 ; i < c->leafp.pheader.numOfKeys ; i++)
		if(key == c->leafp.records[i].key)
			break;

	if(i == c->leafp.pheader.numOfKeys)
		return FAIL;
	
	return offset;	
}
node* dbdelete(node * root, dbint key, int p)
{
	node * key_leaf;
	Records* key_record;
	pagenum_t poffset;

	key_record = dbfind(rootpage[p], key, &poffset,p);
	key_leaf = find_leaf(rootpage[p], key, &poffset,p);

	if(key_record != NULL && key_leaf != NULL)
	{
		root=delete_entry(key_leaf,key,key_record ,poffset,p);
		free(key_record);
		free(key_leaf);

		return root;
	}
	return NULL;
}

node * delete_entry(node * n, dbint key, void * pointer,pagenum_t offset, int p)
{
	node * neighbor,* temp;
	int neighbor_index;
	pagenum_t neighbor_offset;

	n = remove_entry_from_node(n,key,pointer,offset,p);

	if(n->internalp.pheader.parent_page == 0)
		return adjust_root(n,p);

	if(n->internalp.pheader.numOfKeys != 0)
		return rootpage[p];
	
	neighbor_index = get_neighbor_index(n, offset,&neighbor_offset,p);
	temp = coalesce_nodes(n ,offset,neighbor_offset ,neighbor_index,p);

	return rootpage[p];
}

int get_neighbor_index(node * n,pagenum_t offset, pagenum_t * neighbor_offset,int p)
{
	int i;
	node temp;
	
	buf_read_page(n->internalp.pheader.parent_page/PAGESIZE, &temp,p);	

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

node * adjust_root(node * n, int p)
{
	node * root = rootpage[p];
	pagenum_t newRoffset ,oldRoffset;

	if(root->leafp.pheader.numOfKeys > 1)
		return root;

	if(!root->leafp.pheader.is_leaf)
	{
		if((root->internalp.pheader.other_page_offset == 0) && (root->internalp.entities[0].offset == 0))
		{
			buf_flush_page(header[p]->headerp.rootp_offset/PAGESIZE ,p);
			free(rootpage[p]);
			rootpage[p] = NULL;
			chgheadroff(0,p);

			return NULL;
		}			

		if((root->internalp.pheader.other_page_offset == 0) || (root->internalp.entities[0].offset == 0))
		{
			newRoffset = (root->internalp.pheader.other_page_offset == 0) ? root->internalp.entities[0].offset : root->internalp.pheader.other_page_offset;
			oldRoffset = header[p]->headerp.rootp_offset;
			chgheadroff(newRoffset/PAGESIZE,p);		
			buf_flush_page(oldRoffset/PAGESIZE,p);
			buf_read_page(newRoffset/PAGESIZE, rootpage[p],p);
			rootpage[p]->internalp.pheader.parent_page = 0;
			buf_write_page(newRoffset/PAGESIZE, rootpage[p],p);

			return root;
		}		
	}
	else
	{
		if(root->leafp.pheader.numOfKeys == 1)
			return rootpage[p];

		buf_flush_page(header[p]->headerp.rootp_offset/PAGESIZE,p);
		header[p]->headerp.rootp_offset = 0;
		buf_write_page(0,header[p],p);
		free(rootpage[p]);
		rootpage[p] = NULL;
	}

	return rootpage[p];
}

node * remove_entry_from_node(node * n, dbint key, void * pointer,pagenum_t offset, int p)
{
	int i,j, num_pointers;

	i = 0;
	if(n->leafp.pheader.is_leaf)
	{
		while(n->leafp.records[i].key != key)
			i++;
		for(++i; i < n->leafp.pheader.numOfKeys; i++)
		{
			n->leafp.records[i - 1].key = n->leafp.records[i].key;
			Insert_value(n->leafp.records[i - 1].values,p,&n->leafp.records[i]);
		}
		while(i < LBRFACTOR - 1)
		{
			n->leafp.records[i].key = 0;
			for(j = 0 ; j < numOfCol[j] - 1 ; j++)
				n->leafp.records[i].values[j] = 0;
			i++;
		}
	}
	else
	{
		n->internalp.pheader.other_page_offset = 0;
		n->internalp.entities[0].offset = 0;
	}
	n->internalp.pheader.numOfKeys--;
	buf_write_page(offset/PAGESIZE, n, p);

	return n;
}

node * coalesce_nodes(node * n, pagenum_t curoffset, pagenum_t noffset, int n_index, int p)
{
	int i,j;
	pagenum_t temp_off,poffset;
	node *parent = (node*)malloc(sizeof(node));
	node *forfree;
	node temp;

	buf_read_page(n->internalp.pheader.parent_page/PAGESIZE, parent, p);

	poffset = n->internalp.pheader.parent_page;

	buf_flush_page(curoffset/PAGESIZE,p);

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
		buf_write_page(poffset/PAGESIZE, parent, p);
		free(parent);
		return rootpage[p];
	}

	if(parent->internalp.pheader.numOfKeys == 1 && (parent->internalp.entities[0].offset == 0 || parent->internalp.pheader.other_page_offset == 0))
	{
		rootpage[p] = delete_entry(parent,parent->internalp.entities[0].key,n,n->internalp.pheader.parent_page,p);
		free(parent);

		return rootpage[p]; 
	}
	
	if(n_index == -2)
	{
		temp_off = parent->internalp.pheader.other_page_offset;
		parent->internalp.pheader.other_page_offset = 0;
	
		buf_write_page(poffset/PAGESIZE, parent, p);

		adjust_root(parent,p);
		free(parent);
		return rootpage[p];
	}
	else if(n_index == -1)
	{
		temp_off = parent->internalp.entities[0].offset;
		parent->internalp.entities[0].offset = 0;
		
		buf_write_page(poffset/PAGESIZE, parent, p);

		adjust_root(parent,p);
		free(parent);
		return rootpage[p];
	}
}
