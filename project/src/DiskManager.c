#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defines.h"
#include "globals.h"
#include "DiskManager.h"

int make_new_header(int i, int num_col)
{
	Page * newHeader = (Page*)malloc(sizeof(Page));
	newHeader->headerp.freep_offset = 0;
	newHeader->headerp.rootp_offset = 0;
	newHeader->headerp.numOfCol = num_col;
	totalp[i] = newHeader->headerp.numOfPage = 1;

	fseek(default_file[i],0,SEEK_SET);
	fwrite(newHeader,PAGESIZE,1,default_file[i]);
	//fsync(default_fd[i]);
	free(newHeader);

	return 0;
}

int chgheadroff(pagenum_t rootpnum,int i)
{
	header[i]->headerp.rootp_offset = rootpnum * PAGESIZE;
	file_write_page(0,header[i],i);

	return 0;
}

pagenum_t file_alloc_page(int i)
{
	char t = '\0';
	Page * freepp;

	if(header[i]->headerp.freep_offset== 0)
	{
		fseek(default_file[i],0,SEEK_END);
		fwrite(&t,1,PAGESIZE,default_file[i]);
		//fsync(default_fd[i]);
		header[i]->headerp.numOfPage=++totalp[i];
		file_write_page(0,header[i],i);
		currentp[i]=totalp[i] - 1;

		fseek(default_file[i],0,SEEK_END);

		return currentp[i];
	}
	else
	{
		currentp[i] = header[i]->headerp.freep_offset / PAGESIZE;
		freepp = (Page*)malloc(sizeof(Page));		
		file_read_page(currentp[i],freepp,i);
		header[i]->headerp.freep_offset = freepp->freep.next_freep;
		file_write_page(0,header[i],i);
		free(freepp);
		return currentp[i];
	}
}

void file_free_page(pagenum_t pagenum,int i)
{
	Page tempfree;

	if(pagenum > totalp[i])
		exit(1);

	fseek(default_file[i], pagenum * PAGESIZE,SEEK_SET);
	memset(&tempfree,0,PAGESIZE);
	tempfree.freep.next_freep = header[i]->headerp.freep_offset;	


	header[i]->headerp.freep_offset = pagenum * PAGESIZE;
	file_write_page(0,header[i],i);
	fseek(default_file[i], pagenum*PAGESIZE, SEEK_SET);
	fwrite(&tempfree,PAGESIZE,1,default_file[i]);
	//fsync(default_fd[i]);
}

void file_read_page(pagenum_t pagenum, Page* dest, int i)
{
	if(pagenum - 1> totalp[i])
		exit(1);

	fseek(default_file[i], pagenum*PAGESIZE, SEEK_SET);
	fread(dest,PAGESIZE,1,default_file[i]);
}

void file_write_page(pagenum_t pagenum, const Page* src, int i)
{
	Page * freepp;
	if(pagenum - 1> totalp[i])
		exit(1);

	fseek(default_file[i], pagenum*PAGESIZE, SEEK_SET);
	fwrite(src,PAGESIZE,1,default_file[i]);
	//fsync(default_fd[i]);

	if(pagenum != 0 &&( pagenum == header[i]->headerp.rootp_offset / PAGESIZE))
		file_read_page(pagenum, rootpage[i],i);
}
