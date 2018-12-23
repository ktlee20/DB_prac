#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "dbbpt.h"
#include "dbinit.h"
#include "DiskManager.h"
#include "BufferManager.h"
#include "FileIndexManager.h"
#include "Optimizer.h"
#include "LockManager.h"
#include "globals.h"

int init_db(int num_buf)
{
	int i,j;

	if(num_buf < 2)
		num_buf = 2;

	default_file = (FILE**)malloc(sizeof(FILE*) *(MAXTABLE+1));
	default_fd = (int*)malloc(sizeof(int) * (MAXTABLE+1));
	header = (Page**)malloc(sizeof(Page*) * (MAXTABLE+1));
	rootpage = (Page**)malloc(sizeof(Page*) * (MAXTABLE+1));
	totalp = (pagenum_t*)malloc(sizeof(pagenum_t) * (MAXTABLE+1));
	currentp = (pagenum_t*)malloc(sizeof(pagenum_t) * (MAXTABLE+1));

	default_file[0] = (FILE*)0x01;
	default_fd[0] = 100000;
	totalp[0] = -1;
	currentp[0] = -1;
	header[0] = (Page*)0x01;
	rootpage[0] = (Page*)0x01;
	oinfo[0] = (OptInfo*)0x01;
	memory_key[0] = (JTable*)0x01;

	bufctrl[0].buffer = (Bufstrt*)0x01;
	bufctrl[0].bufferlast = (Bufstrt*)0x01;
	bufctrl[0].curPos = -1;	
	
	txnNumber = 0;
	txnlist.head = txnlist.tail = NULL;
	pthread_mutex_init(&txnlist.txn_list_mutex, NULL);

	for(i = 1 ; i < (MAXTABLE + 1) ; i++)
	{
		globalInit(i);
		newBuf(num_buf, i);
	}

	for(i = 0 ; i < MAXJOINNUM + 1 ; i++)
		for(j = 0 ; j < MAXVALNUM + 1 ; j++)
			thread_t[i][j] = 0xFFFFFFFFFFFFFFFF;
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
	for(i = 1 ; i <= MAXTABLE ; i++)
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

dbint * find(int table_id, dbint key, int tid, int * result)
{
	dbint page_id = get_leaf_page_id(rootpage[table_id],key,table_id);
	if(page_id == FAIL)
		return NULL;

	lock_t * lock = insert_lock_table(page_id, tid, table_id,__sync_fetch_and_add(&timer[table_id], 1), SHARED);
	Page * page;	
	int i,j;
	dbint * value;

	if(lock == NULL)
	{
		*result = FAILED;
		pthread_mutex_unlock(&bufctrl[table_id].buf_ctrl_mutex);
		return NULL;
	}	
	
	pthread_mutex_lock(&lock->lock_mutex);
	while(lock->txn->mode != RUNNING)
	{
		pthread_cond_wait(&lock->cond, &lock->lock_mutex);
	}
	pthread_mutex_unlock(&lock->lock_mutex);
	page = &lock->buffer->frame;	

	for(i = 0 ; i < page->leafp.pheader.numOfKeys ; i++)
		if(page->leafp.records[i].key == key)
			break;

	value = (dbint*)malloc(sizeof(dbint) * numOfCol[table_id]);

	value[0] = page->leafp.records[i].key;
	for(j = 1 ; j < numOfCol[table_id] ; j++)
		value[j] = page->leafp.records[i].values[j - 1];	

	*result = SUCCESS;
	
	pthread_mutex_unlock(&bufctrl[table_id].buf_ctrl_mutex);
	return value;
		
}

int update(int table_id, dbint key, dbint * values, int tid, int * result)
{
	dbint page_id = get_leaf_page_id(rootpage[table_id],key,table_id);
	if(page_id == FAIL)
		return 0;

	lock_t * lock = insert_lock_table(page_id, tid, table_id, __sync_fetch_and_add(&timer[table_id],1), EXCLUSIVE);
	Page * page;
	dbint * oldvalues, * newvalues;	
	int i,j;
	
	if(lock == NULL)
	{
		*result = FAILED;
		pthread_mutex_unlock(&bufctrl[table_id].buf_ctrl_mutex);
		return 0;
	}

	pthread_mutex_lock(&lock->lock_mutex);
	while(lock->txn->mode != RUNNING)
	{
		pthread_cond_wait(&lock->cond, &lock->lock_mutex);
	}
	pthread_mutex_unlock(&lock->lock_mutex);
	page = &lock->buffer->frame;

	for(i = 0 ; i < page->leafp.pheader.numOfKeys ; i++)
		if(page->leafp.records[i].key == key)
			break;

	oldvalues = (dbint*)malloc(sizeof(dbint) * numOfCol[table_id]);
	newvalues = (dbint*)malloc(sizeof(dbint) * numOfCol[table_id]);

	oldvalues[0] = page->leafp.records[i].key;
	newvalues[0] = key;
	
	for(j = 1 ; j < numOfCol[table_id] ; j++)
		oldvalues[j] = page->leafp.records[i].values[j - 1];


	page->leafp.records[i].key = key;	
		
	for(j = 0 ; j < numOfCol[table_id] - 1 ; j++)
		newvalues[j + 1] = page->leafp.records[i].values[j] = values[j];

	putLog(lock->txn, table_id, page_id, key, oldvalues, newvalues);
	
	*result = SUCCESS;
	pthread_mutex_unlock(&bufctrl[table_id].buf_ctrl_mutex);
	return 1;
}

dbint* no_th_find(int table_id, dbint key)
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
