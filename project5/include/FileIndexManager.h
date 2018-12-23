#ifndef __INDEX_FILE_MANAGER_
#define __INDEX_FILE_MANAGER_

#include "defines.h"

int init_db(int);
int open_table(char * pathname,int num_column);
int close_table(int);
int shutdown_db();
int insert(int table_id, dbint key, dbint* values);
dbint * find(int table_id, dbint key, int tid, int * result);
int update(int table_id, dbint key, dbint * values, int tid, int * result);
dbint* no_th_find(int table_id, dbint key);
int erase(int table_id, dbint key);
pagenum_t getPageId(int table_id, dbint key);

#endif
