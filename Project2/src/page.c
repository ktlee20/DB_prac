#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "defines.h"
#include "globals.h"
#include "page.h"

pagenum_t file_alloc_page()
{
	char t = '\0';
	Page freei;

	if(Header.headerp.freep_offset== 0)
	{
	fseek(default_file,0,SEEK_END);
	fwrite(&t,1,PAGESIZE,default_file);
	totalp++;
	currentp=totalp;

	return currentp;
	}
	else
	{
		file_read_page(Header.headerp.freep_offset, &freei);
		currentp = Header.headerp.freep_offset;
		Header.headerp.freep_offset = freei.freep.next_freep;	
		
		return currentp;
	}
}

void file_free_page(pagenum_t pagenum)
{
	Page tempfree;

	if(pagenum > totalp)
		exit(1);

	fseek(default_file, pagenum * PAGESIZE,SEEK_SET);
	tempfree.freep.next_freep = Header.headerp.freep_offset;
	Header.headerp.freep_offset = pagenum;
	fwrite(&tempfree,PAGESIZE,1,default_file);
}

void file_read_page(pagenum_t pagenum, Page* dest)
{
	if(pagenum > totalp)
		exit(1);

	fseek(default_file, pagenum*PAGESIZE, SEEK_SET);
	fread(dest,PAGESIZE,1,default_file);
}

void file_write_page(pagenum_t pagenum, const Page* src)
{
	if(pagenum > totalp)
		exit(1);

	fseek(default_file, pagenum*PAGESIZE, SEEK_SET);
	fwrite(src,PAGESIZE,1,default_file);
}
