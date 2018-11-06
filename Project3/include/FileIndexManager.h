#ifndef __INDEX_FILE_MANAGER_
#define __INDEX_FILE_MANAGER_

#include "defines.h"

int init_db(int);
int open_table(char * pathname);
int close_table(int);
int shutdown_db();
int insert(int table_id, dbint key, char * value);
char * find(int table_id, dbint key);
int delete(int table_id, dbint key);
pagenum_t getPageId(int table_id, dbint key);

#endif
