#include <stdio.h>
#include <stdlib.h>
#include "dbinit.h"
#include "defines.h"
#include "globals.h"
#include "FileIndexManager.h"
#include "DiskManager.h"

void globalInit()
{
	default_file = NULL;
	default_fd = 0;
	totalp = 0;
	currentp = 0;
	header = NULL;
	rootpage = NULL;
}

int headerInit()
{
	header = (Page*)malloc(sizeof(Page));
	file_read_page(0,header);
	totalp = header->headerp.numOfPage;
	
	if(header->headerp.rootp_offset != 0)
	{
		rootpage = (Page*)malloc(sizeof(Page));
		file_read_page(header->headerp.rootp_offset, rootpage);
	}
	return 0;
}

void dbend()
{
	if(header != NULL);
	{
		file_write_page(0,header);
		free(header);
	}
	if(rootpage != NULL);
		free(rootpage);
	if(default_file != NULL)
		free(default_file);
}

