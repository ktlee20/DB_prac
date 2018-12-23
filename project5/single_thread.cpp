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
	//int *result1, *result2;
	int result,i;
	dbint arr[3] = {0,1,2};
	dbint * temp;

	init_db(128);
	table_id1 = open_table("default.db",4);

	tid = begin_tx();

	update(table_id1, 500, arr, tid, &result);	
	update(table_id1, 1000, arr, tid, &result);	
	update(table_id1, 15000, arr, tid, &result);	
	update(table_id1, 98000, arr, tid, &result);	
	update(table_id1, 99999, arr, tid, &result);	
	update(table_id1, 12, arr, tid, &result);	

	temp = find(table_id1, 500, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 1000, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 15000, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 98000, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 99999, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 12, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	

	printf("txn1 finish\n");
	end_tx(tid);
	
	tid = begin_tx();

	update(table_id1, 500, arr, tid, &result);	
	update(table_id1, 1000, arr, tid, &result);	
	update(table_id1, 15000, arr, tid, &result);	
	update(table_id1, 98000, arr, tid, &result);	
	update(table_id1, 99999, arr, tid, &result);	
	update(table_id1, 12, arr, tid, &result);	

	temp = find(table_id1, 500, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 1000, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 15000, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 98000, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 99999, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	temp = find(table_id1, 12, tid, &result);	
	printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	
	update(table_id1, 7, arr,tid, &result);
	find(table_id1, 7, tid, &result);	
	

	//버퍼풀 부족으로 set의 size를 키우지는 못했습니다.
	for(i = 1 ; i < 100 ; i++)
	{
		update(table_id1, i, arr, tid, &result);	
	}

	for(i = 1 ; i < 200 ; i += 2)
	{
		temp = find(table_id1, i, tid, &result);	
		if(temp == NULL)
			continue;
		printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	}

	for(i = 1 ; i < 100 ; i++)
	{
		update(table_id1, i, arr, tid, &result);	
	}

	for(i = 1 ; i < 200 ; i += 2)
	{
		temp = find(table_id1, i, tid, &result);	
		if(temp == NULL)
			continue;
		printf("%ld %ld %ld %ld\n",temp[0],temp[1],temp[2], temp[3]);
	}
	printf("txn1 finish\n");

	end_tx(tid);
	
	

	/*
	pthread_create(&thd1, NULL, func_txn1, NULL);
	pthread_create(&thd2, NULL, func_txn2, NULL);

	pthread_join(thd1, (void**)(&result1));
	pthread_join(thd2, (void**)(&result2));
	*/
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
