#ifndef __BUFFER_MANAGER_H_
#define __BUFFER_MANAGER_H_

#include "defines.h"

Bufstrt * makeBufNode();
void bufferAttr(Bufstrt* node, dbint offset, int table_id);
void newBuf(int new_buf, int table_id);
void closeBuf(int table_id);
pagenum_t get_page_offset(Bufstrt * node);
Bufstrt * find_leaf_buffer(pagenum_t offset, int table_id);
Page * set_leaf_page(Page * dest, Page * src);
Page * delete_leaf_buffer(Page * dest, dbint key);
Page * insert_into_buffer(Page * dest, dbint key, char * pointer);
void buf_flush_page(pagenum_t offset, int table_id);
Page * buf_read_page(pagenum_t offset, Page * dest ,int table_id);
void buf_write_page(pagenum_t offset, Page * src, int table_id);
void buf_lru_set(Bufstrt * node,int table_id);
Bufstrt * get_buf_node(pagenum_t offset, int table_id);

#endif
