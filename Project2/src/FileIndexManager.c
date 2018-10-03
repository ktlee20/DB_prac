#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "DiskManager.h"
#include "FileIndexManager.h"
#include "globals.h"

int open_db(char * pathname)
{
	rootpage = NULL;
	default_file = fopen(pathname,"r+b");
	if(default_file == NULL)
	{
		default_file = fopen(pathname,"w+b");
		make_new_header();

		if(default_file == NULL)
			return -1;

		return 0;
	}
	header = (Page*)malloc(sizeof(Page));
	file_read_page(0,header);

	if(header->headerp.rootp_offset != 0)
		file_read_page(header->headerp.rootp_offset,rootpage);

	return 0;
}

int insert(dbint key, char * value)
{

}
