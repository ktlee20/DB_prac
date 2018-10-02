#ifndef __DISK_BASE_BPT_H_
#define __DISK_BASE_BPT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "defines.h"

typedef uint64_t dbint;
typedef dbint pagenum_t;
typedef Page node;

pagenum_t findleaf(node * leaf, int key);
char* find(node * leaf, int key);
int findkey(node* leaf, int key);
Records* make_record(char * value);
node * make_leaf(dbint parent_page, dbint other_page_offset);
int start_new_tree(int key, Records* pointer);
void insert_into_leaf(node * leaf,pagenum_t offset, int key, Records* poiner);
int insert_into_leaf_after_splitting(node * leaf, int key, Records* pointer, pagenum_t offset);
int insert(int key, char * value);

#endif
