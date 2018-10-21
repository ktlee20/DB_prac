#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "dbinit.h"
#include "FileIndexManager.h"

FILE * default_file;
Page * header;
Page * rootpage;
pagenum_t totalp;
pagenum_t currentp;

int main(int argc, char * argv[])
{
	int i;
	char * temp;

	open_db("default.db");

	for(i = 0 ; i < 10000000; i++)
		insert(i, "lee");
	for(i = 0 ; i < 10000000; i++)
		find(i);
	for(i = 0 ; i < 10000000; i++)
		delete(i);

	for(i = 0 ; i < 10000000 ; i++)
		insert(i,"lee");

	for(i = 1 ; i < 10000000 ; i+=7)
		delete(i);
	for(i = 0 ; i < 10000000 ; i++)
	{
		if((temp=find(i)) != NULL)
		{
			printf("%d : %s\n",i,temp);
			free(temp);
		}
		else
		{
			printf("Not Found\n!");
		}
	}

	dbend();
	return 0;
}
