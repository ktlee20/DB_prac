#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "dbbpt.h"
#include "dbinit.h"
#include "DiskManager.h"
#include "BufferManager.h"
#include "FileIndexManager.h"
#include "Optimizer.h"
#include "globals.h"

int init_db(int num_buf)
{
	int i;

	if(num_buf < 2)
		num_buf = 2;

	default_file = (FILE**)malloc(sizeof(FILE*) * MAXTABLE);
	default_fd = (int*)malloc(sizeof(int) * MAXTABLE);
	header = (Page**)malloc(sizeof(Page*) * MAXTABLE);
	rootpage = (Page**)malloc(sizeof(Page*) * MAXTABLE);
	totalp = (pagenum_t*)malloc(sizeof(pagenum_t) * MAXTABLE);
	currentp = (pagenum_t*)malloc(sizeof(pagenum_t) * MAXTABLE);

	for(i = 0 ; i < MAXTABLE ; i++)
	{
		globalInit(i);
		newBuf(num_buf, i);
	}
}

int open_table(char * pathname, int num_column)
{
	int num = empty();
	
	if(num == -1)
		exit(1);

	if(!(num_column >= 2 && num_column <= 16))
	{
		printf("please enter a right column number(2 <= colnum <= 16)\n");
		exit(1);
	}

	default_file[num] = fopen(pathname,"r+b");
	numOfCol[num] = num_column;

	if(default_file[num] != NULL)
		default_fd[num] = fileno(default_file[num]);

	if(default_file[num] == NULL)
	{
		default_file[num] = fopen(pathname,"w+b");
		default_fd[num] = fileno(default_file[num]);
		make_new_header(num, num_column);

		if(default_file[num] == NULL)
			return -1;

		headerInit(num,num_column);
		return num;
	}
	headerInit(num,num_column);

	if(header[num]->headerp.rootp_offset != 0)
		file_read_page(header[num]->headerp.rootp_offset,rootpage[num],num);

	make_stat(num);
	return num;
}

int close_table(int table_id)
{
	closeBuf(table_id);
	dbend(table_id);
	clearele(table_id);
}

int shutdown_db()
{
	int i;
	for(i = 0 ; i < MAXTABLE ; i++)
	{
		if(default_fd[i] != 0)
			close_table(i);
	}
	free(default_file);
	free(default_fd);
	free(header);
	free(rootpage);
	free(totalp);
	free(currentp);

	return 0;
}

int insert(int table_id,dbint key, dbint* values)
{
	if(rootpage[table_id] == dbinsert(rootpage[table_id],key,values,table_id))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

dbint* find(int table_id, dbint key)
{
	int i;
	pagenum_t leaf_offset;
	Records * record = dbfind(rootpage[table_id], key, &leaf_offset,table_id);
	dbint * value= (dbint*)malloc(sizeof(dbint) *numOfCol[table_id]); 

	if(record != NULL)
	{
		for(i = 0 ; i < numOfCol[table_id] - 1; i++)
			value[i] = record->values[i];
	}		
	else
	{
		free(value);
		value = NULL;
	}
	free(record);

	return value;
}

pagenum_t getPageId(int table_id, dbint key)
{
	pagenum_t leaf_offset;
	dbfind(rootpage[table_id], key, &leaf_offset, table_id);
	
	return leaf_offset;
}

int erase(int table_id,dbint key)
{
	if(rootpage[table_id] == dbdelete(rootpage[table_id], key,table_id))
		return 0;
	else
		return -1;

}
