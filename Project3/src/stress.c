#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "dbinit.h"
#include "FileIndexManager.h"

int main(int argc, char * argv[])
{
	int i;
	char * temp;
	int k;	

	init_db(1);
	//init_db(100);
	k = open_table("default.db");

	for(i = 0 ; i < 10000; i++)
		insert(k,i, "lee");
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		delete(k,i);

	for(i = 0 ; i < 10000 ; i++)
		insert(k,i,"lee");

	for(i = 1 ; i < 10000 ; i+=7)
		delete(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %s\n",i,temp);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	
	close_table(k);
	k = open_table("default1.db");

	for(i = 0 ; i < 10000; i++)
		insert(k,i, "lee");
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		delete(k,i);

	for(i = 0 ; i < 10000 ; i++)
		insert(k,i,"lee");

	for(i = 1 ; i < 10000 ; i+=7)
		delete(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %s\n",i,temp);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	

	k = open_table("default2.db");

	for(i = 0 ; i < 10000; i++)
		insert(k,i, "lee");
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		delete(k,i);

	for(i = 0 ; i < 10000 ; i++)
		insert(k,i,"lee");

	for(i = 1 ; i < 10000 ; i+=7)
		delete(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %s\n",i,temp);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	
	k = open_table("default3.db");

	for(i = 0 ; i < 10000; i++)
		insert(k,i, "lee");
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		delete(k,i);

	for(i = 0 ; i < 10000 ; i++)
		insert(k,i,"lee");

	for(i = 1 ; i < 10000 ; i+=7)
		delete(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %s\n",i,temp);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}
	
	close_table(k);
	k = open_table("default4.db");

	for(i = 0 ; i < 10000; i++)
		insert(k,i, "lee");
	for(i = 0 ; i < 10000; i++)
		find(k,i);
	for(i = 0 ; i < 10000; i++)
		delete(k,i);

	for(i = 0 ; i < 10000 ; i++)
		insert(k,i,"lee");

	for(i = 1 ; i < 10000 ; i+=7)
		delete(k,i);
	for(i = 0 ; i < 10000 ; i++)
	{
		if((temp=find(k,i)) != NULL)
		{
			printf("%d : %s\n",i,temp);
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
