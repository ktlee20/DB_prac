#ifndef __PAGE_H_
#define __PAGE_H_

#include "defines.h"

int make_new_header(int);
int chgheadroff(pagenum_t rootpnum,int i);
pagenum_t file_alloc_page(int);
void file_free_page(pagenum_t pagenum, int i);
void file_read_page(pagenum_t pagenum, Page * dest,int i);
void file_write_page(pagenum_t pagenum, const Page * src,int i);

#endif
