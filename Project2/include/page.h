#ifndef __PAGE_H_
#define __PAGE_H_

#include "defines.h"

void dbStart();
void headerInit();
void dbEnd();
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);

#endif
