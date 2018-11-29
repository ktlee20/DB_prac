#ifndef __PAGE_H_
#define __PAGE_H_

#include "defines.h"

int make_new_header();
int chgheadroff(pagenum_t rootpnum);
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
void file_read_page(pagenum_t pagenum, Page * dest);
void file_write_page(pagenum_t pagenum, const Page * src);

#endif
