#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "dbinit.h"
#include "FileIndexManager.h"
#include "join.h"
#include "globals.h"

int main(int argc, char * argv[])
{
	int i;
	dbint * temp;
	dbint arr[3] = {1,2,3};
	dbint arr1[4] = {1,2,3,4};
	dbint arr2[1] = {1};
	dbint arr3[2] ={1,2};
	int k;	

	printJTable = 1;

	init_db(100);
	k = open_table("default.db",4);
	k = open_table("default1.db",5);
	k = open_table("default2.db",2);
	k = open_table("default3.db",3);
	k = open_table("default4.db",4);

	join("0.0=1.0&1.0=2.0&2.0=3.0&3.0=4.0");	

	shutdown_db();
	return 0;
}
