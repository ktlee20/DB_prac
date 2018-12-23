#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "libbpt.h"

void set_global_state(int s);
int get_global_state();
void * func_txn1(void * arg);
void * func_txn2(void * arg);

int global_state = 0;
pthread_t thd1, thd2;
int table_id1, table_id2;

int main(int argc, char * argv[])
{
	int tid;
	int *result1, *result2;

	init_db(128);
	table_id1 = open_table("default.db",4);

	pthread_create(&thd1, NULL, func_txn1, NULL);
	pthread_create(&thd2, NULL, func_txn2, NULL);

	pthread_join(thd1, (void**)(&result1));
	pthread_join(thd2, (void**)(&result2));
	shutdown_db();
	return 0;
}

void * func_txn1(void * arg)
{
	int tid;
	int * result = (int*)malloc(sizeof(int));
	tid = begin_tx();
	dbint arr[3] = {0,1,2};
	update(table_id1, 500, arr, tid, result);	
	set_global_state(1);
	sleep(10);
	printf("txn1 finish\n");
	end_tx(tid);

	return (void*)result;
}

void * func_txn2(void * arg)
{
	int tid = begin_tx();
	int * result = (int*)malloc(sizeof(int));
	dbint arr[3] = {0,1,2};
	while(get_global_state() == 0);
	update(table_id1, 500, arr, tid, result);
	printf("txn2 finish\n");
	end_tx(tid);	

	return (void*)result;
}

void set_global_state(int s)
{
	global_state = s;
	__sync_synchronize();
}

int get_global_state()
{
	__sync_synchronize();
	return global_state;
}
