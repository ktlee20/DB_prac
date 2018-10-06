#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "dbbpt.h"
#include "dbinit.h"
#include "DiskManager.h"
#include "FileIndexManager.h"
#include "globals.h"

int open_db(char * pathname)
{
	default_file = fopen(pathname,"r+b");
	if(default_file == NULL)
	{
		default_file = fopen(pathname,"w+b");
		make_new_header();

		if(default_file == NULL)
			return -1;

		headerInit();
		return 0;
	}
	header = (Page*)malloc(sizeof(Page));
	file_read_page(0,header);
	headerInit();

	if(header->headerp.rootp_offset != 0)
		file_read_page(header->headerp.rootp_offset,rootpage);

	return 0;
}

int insert(dbint key, char * value)
{
	if(rootpage == dbinsert(rootpage,key,value))
		return 0;
	else
		return -1;
}

char * find(dbint key)
{
	pagenum_t leaf_offset;
	Records * record = dbfind(rootpage, key, &leaf_offset);
	char * value = (char*)malloc(sizeof(char) * VALUESIZE);

	printf("%ld ",key);
	strcpy(value, record->value);

	free(record);

	return value;
}

int delete(dbint key)
{
	if(rootpage == dbdelete(rootpage, key))
		return 0;
	else
		return -1;

}
