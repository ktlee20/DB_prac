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

	globalInit();
	open_db("default.db");
	for(i = 0 ; i < 100000; i++)
		insert(i,"lee");
	for(i = 0 ; i < 100000 ; i++)
		printf("%s\n", find(i));
	for(i = 0 ; i < 100000 ; i++)
		delete(i);

	for(i = 1 ; i < 100000 ; i += 2)
		insert(i, "lee");
	for(i = 99999; i > 0 ; i -= 2)
		delete(i);

	for(i = 3 ; i < 10000; i += 3)
		insert(i, "lee");

	for(i = 4 ; i < 15000 ; i += 4)
		insert(i , "lee");
	for(i = 3 ; i <10000 ; i += 3)
		printf("%s\n",find(i));

	dbend();
	return 0;
}
