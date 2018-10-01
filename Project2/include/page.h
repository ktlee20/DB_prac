#ifndef __PAGE_H_
#define __PAGE_H_

#include "defines.h"

pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
void file_read_page(pagenum_t pagenum, Page * dest);
void file_write_page(pagenum_t pagenum, const Page * src);

#endif
