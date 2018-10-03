#ifndef __DISK_BASE_BPLUSTREE_
#define __DISK_BASE_BPLUSTREE_

#include "defines.h"

typedef Page node;
node * dbinsert(node * root, int key, char * value);
Records * dbfind(node * root, int key,pagenum_t* poffset);
node * find_leaf(node * root, int key,pagenum_t* poffset);
Records * make_records(char * value);
node * start_new_tree(int key, Records* pointer);
node * make_leaf(void);
node * make_node(void);
node * insert_into_leaf(node * leaf, int key, Records* pointer,pagenum_t offset);
node * insert_into_leaf_after_splitting(node * leaf, int key, Records * pointer, pagenum_t old_offset);
int get_left_index(node * parent, pagenum_t loffset);
node * insert_into_new_root(node * left, pagenum_t loffset, int key, node * right, pagenum_t roffset);
node * insert_into_node(node *n, int left_index, int key, node * right, pagenum_t roffset);
node * insert_into_parent(node * left, pagenum_t loffset, int key, node * right, pagenum_t roffset);
node * insert_into_node_after_splitting(node * old_node, pagenum_t old_offset, int left_index, int key, node * right, pagenum_t roffset);
node * dbdelete(node * root, int key);
node * delete_entry(node * n, int key, void * pointer, pagenum_t offset);
int get_neighbor_index(node * n, pagenum_t offset, pagenum_t * neighbor_offset);
node * adjust_root(node * root);
node * remove_entry_from_node(node * n, int key, node * pointer, pagenum_t offset);
node * coalesce_nodes(node * n, pagenum_t curoffset, pagenum_t noffset, int n_index);
#endif
