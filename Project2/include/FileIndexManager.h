#ifndef __INDEX_FILE_MANAGER_
#define __INDEX_FILE_MANAGER_

#include "defines.h"

int open_db(char * pathname);
int insert(dbint key, char * value);
char * find(dbint key);
int delete(dbint key);

#endif
