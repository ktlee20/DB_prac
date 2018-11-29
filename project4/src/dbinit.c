#include <stdio.h>
#include <stdlib.h>
#include "dbinit.h"
#include "defines.h"
#include "globals.h"
#include "FileIndexManager.h"
#include "DiskManager.h"

FILE ** default_file;
int * default_fd;

void globalInit(int i)
{
	default_file[i] = NULL;
	default_fd[i] = 0;
	totalp[i] = 0;
	currentp[i] = 0;
	header[i] = NULL;
	rootpage[i] = NULL;
	oinfo[i] = NULL;
	memory_key[i] = NULL;
}

int empty()
{	
	int i;
	for(i = 0 ; i < MAXTABLE ; i++)
		if(default_fd[i] == 0)
			return i;

	return -1;
}

int clearele(int table_id)
{
	globalInit(table_id);
}

int headerInit(int i,int num_col)
{
	header[i] = (Page*)malloc(sizeof(Page));
	file_read_page(0,header[i],i);
	totalp[i] = header[i]->headerp.numOfPage;
	
	if(num_col != header[i]->headerp.numOfCol)
	{
		//printf("Warning!! your num_col and header's numofCol is different. column number is set to header's number\n");
		numOfCol[i] = header[i]->headerp.numOfCol;
	}	

	if(header[i]->headerp.rootp_offset != 0)
	{
		rootpage[i] = (Page*)malloc(sizeof(Page));
		file_read_page((header[i]->headerp.rootp_offset/PAGESIZE), rootpage[i],i);
	}
	return 0;
}

void dbend(int i)
{
	dbint j;

	if(oinfo[i] != NULL)
	{
		free(oinfo[i]);
		oinfo[i] = NULL;
	}
	if(header[i] != NULL);
	{
		file_write_page(0,header[i],i);
		free(header[i]);
		header[i] = NULL;
	}
	if(rootpage[i] != NULL);
	{
		free(rootpage[i]);
		rootpage[i] = NULL;
	}
	if(default_file[i] != NULL)
	{
		fclose(default_file[i]);
		default_file[i] = NULL;
	}
	if(memory_key[i] != NULL)
	{
		for(j = 0 ; j < memory_key[i]->numOfData ; j++)
			free(memory_key[i]->iArr[j]);
		free(memory_key[i]);
		memory_key[i] = NULL;
	}
	numOfCol[i] = 0;
}

