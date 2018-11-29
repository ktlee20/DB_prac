#ifndef __DISK_BASE_BPLUSTREE_
#define __DISK_BASE_BPLUSTREE_

#include "defines.h"

typedef Page node;
node * dbinsert(node * root, dbint key, char * value);
Records * dbfind(node * root, dbint key,pagenum_t* poffset);
node * find_leaf(node * root, dbint key,pagenum_t* poffset);
Records * make_records(char * value);
node * start_new_tree(dbint key, Records* pointer);
node * make_leaf(pagenum_t parent_off, pagenum_t otherp_off);
node * make_node(pagenum_t parent_off, pagenum_t otherp_off);
node * insert_into_leaf(node * leaf, dbint key, Records* pointer,pagenum_t offset);
node * insert_into_leaf_after_splitting(node * leaf, dbint key, Records * pointer, pagenum_t old_offset);
int get_left_index(node * parent, pagenum_t loffset);
node * insert_into_new_root(node * left, pagenum_t loffset, dbint key, node * right, pagenum_t roffset);
node * insert_into_node(node *n, int left_index, dbint key, node * right, pagenum_t roffset);
node * insert_into_parent(node * left, pagenum_t loffset, dbint key, node * right, pagenum_t roffset);
node * insert_into_node_after_splitting(node * old_node, pagenum_t old_offset, int left_index, dbint key, node * right, pagenum_t roffset);
node * dbdelete(node * root, dbint key);
node * delete_entry(node * n, dbint key, void * pointer, pagenum_t offset);
int get_neighbor_index(node * n, pagenum_t offset, pagenum_t * neighbor_offset);
node * adjust_root(node * root);
node * remove_entry_from_node(node * n, dbint key, void * pointer, pagenum_t offset);
node * coalesce_nodes(node * n, pagenum_t curoffset, pagenum_t noffset, int n_index);
pagenum_t lowestkey(pagenum_t offset,node ** pforfree);
#endif
