#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "defines.h"
#include "dbinit.h"
#include "FileIndexManager.h"

int main(int argc, char * argv[])
{
	int i,j;
	dbint * temp;
	srand(time(NULL));
	dbint arr[3] = {1,2,3};
	dbint arr1[4] = {1,2,3,4};
	dbint arr2[1] = {1};
	dbint arr3[2] ={1,2};
	int k;	

	init_db(100);
	k = open_table("default.db",4);

	for(i = 0 ; i < 100000; i++)
	{
		for(j = 0 ; j < 3 ; j++)
			arr[j] = rand() % 1000;
		insert(k,i, arr);
	}
	for(i = 0 ; i < 100000; i++)
		find(k,i);
	for(i = 0 ; i < 100000; i++)
		erase(k,i);

	for(i = 0 ; i < 100000 ; i++)
	{
		for(j = 0 ; j < 3 ; j++)
			arr[j] = rand()%1000;
		insert(k,i,arr);
	}
	for(i = 1 ; i < 100000 ; i+=7)
		erase(k,i);
	for(i = 0 ; i < 100000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %ld %ld %ld\n",i,temp[0],temp[1],temp[2]);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	
	close_table(k);
	
	k = open_table("default1.db",5);

	for(i = 0 ; i < 100000; i++)
	{
		for(j = 0 ; j < 4 ; j++)
			arr1[j] = rand()%1000;
		insert(k,i, arr1);
	}
	for(i = 0 ; i < 100000; i++)
		find(k,i);
	for(i = 0 ; i < 100000; i++)
		erase(k,i);

	for(i = 0 ; i < 100000 ; i++)
	{
		for(j = 0 ; j < 3 ; j++)
			arr1[j] = rand()%1000;
		insert(k,i,arr1);
	}
	for(i = 1 ; i < 100000 ; i+=7)
		erase(k,i);
	for(i = 0 ; i < 100000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %ld %ld %ld %ld\n",i,temp[0],temp[1],temp[2],temp[3]);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	

	k = open_table("default2.db",2);

	for(i = 0 ; i < 100000; i++)
	{
		arr2[0] = rand() % 1000;
		insert(k,i, arr2);
	}
	for(i = 0 ; i < 100000; i++)
		find(k,i);
	for(i = 0 ; i < 100000; i++)
		erase(k,i);

	for(i = 0 ; i < 100000 ; i++)
	{
		arr2[0] = rand() % 1000;
		insert(k,i,arr2);
	}
	for(i = 1 ; i < 100000 ; i+=7)
		erase(k,i);
	for(i = 0 ; i < 100000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %ld\n",i,temp[0]);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	
	k = open_table("default3.db",3);

	for(i = 0 ; i < 10000; i++)
	{
		for(j = 0 ; j < 2 ; j++)
			arr3[j] = rand()%1000;
		insert(k,i, arr3);
	}
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		erase(k,i);

	for(i = 0 ; i < 10000 ; i++)
	{
		for(j = 0 ; j < 2 ; j++)
			arr3[j] = rand()%1000;
		insert(k,i,arr3);
	}

	for(i = 1 ; i < 10000 ; i+=7)
		erase(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %ld %ld \n",i,temp[0],temp[1]);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	
	close_table(k);
	k = open_table("default4.db",4);

	for(i = 0 ; i < 10000; i++)
	{
		for(j = 0 ; j < 3 ; j++)
			arr[j] = rand()%1000;
		insert(k,i, arr);
	}
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		erase(k,i);

	for(i = 0 ; i < 10000 ; i++)
	{
		for(j = 0 ; j < 3 ; j++)
			arr[j] = rand()%1000;
		insert(k,i,arr);
	}
	for(i = 1 ; i < 10000 ; i+=7)
		erase(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %ld %ld %ld\n",i,temp[0],temp[1],temp[2]);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	shutdown_db();
	return 0;
}
