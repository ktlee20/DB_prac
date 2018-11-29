#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defines.h"
#include "globals.h"
#include "DiskManager.h"

int make_new_header()
{
	Page * newHeader = (Page*)malloc(sizeof(Page));
	newHeader->headerp.freep_offset = 0;
	newHeader->headerp.rootp_offset = 0;
	totalp = newHeader->headerp.numOfPage = 1;

	fseek(default_file,0,SEEK_SET);
	fwrite(newHeader,PAGESIZE,1,default_file);
	fsync(default_fd);
	free(newHeader);

	return 0;
}

int chgheadroff(pagenum_t rootpnum)
{
	header->headerp.rootp_offset = rootpnum;
	file_write_page(0,header);

	return 0;
}

pagenum_t file_alloc_page()
{
	char t = '\0';
	int size;

	if(header->headerp.freep_offset== 0)
	{
		fseek(default_file,0,SEEK_END);
		fwrite(&t,1,PAGESIZE,default_file);
		fsync(default_fd);
		header->headerp.numOfPage=++totalp;
		file_write_page(0,header);
		currentp=totalp - 1;

		fseek(default_file,0,SEEK_END);
		size = ftell(default_file);	

		return currentp;
	}
	else
	{
		currentp = header->headerp.freep_offset;
		return currentp;
	}
}

void file_free_page(pagenum_t pagenum)
{
	Page tempfree;

	if(pagenum > totalp)
		exit(1);

	fseek(default_file, pagenum * PAGESIZE,SEEK_SET);
	tempfree.freep.next_freep = header->headerp.freep_offset;
	header->headerp.freep_offset = pagenum;
	file_write_page(0,header);
	fseek(default_file, pagenum*PAGESIZE, SEEK_SET);
	fwrite(&tempfree,PAGESIZE,1,default_file);
	fsync(default_fd);
}

void file_read_page(pagenum_t pagenum, Page* dest)
{
	if(pagenum - 1> totalp)
		exit(1);

	fseek(default_file, pagenum*PAGESIZE, SEEK_SET);
	fread(dest,PAGESIZE,1,default_file);
}

void file_write_page(pagenum_t pagenum, const Page* src)
{
	Page * freepp;
	if(pagenum - 1> totalp)
		exit(1);

	if(pagenum != 0 && pagenum == header->headerp.freep_offset)
	{
		freepp = (Page*)malloc(sizeof(Page));		
		file_read_page(header->headerp.freep_offset,freepp);
		header->headerp.freep_offset = freepp->freep.next_freep;
		file_write_page(0,header);
		free(freepp);
	}

	fseek(default_file, pagenum*PAGESIZE, SEEK_SET);
	fwrite(src,PAGESIZE,1,default_file);
	fsync(default_fd);

	if(pagenum != 0 && pagenum == header->headerp.rootp_offset)
		file_read_page(pagenum, rootpage);
}
