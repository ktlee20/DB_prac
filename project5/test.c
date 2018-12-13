#include <stdlib.h>
#include <string.h>
#include "libbpt.h"

int main(int argc, char * argv[])
{
	int i;
	FILE * fp;
	dbint * temp;
	dbint arr[3] = {1,2,3};
	dbint arr1[4] = {1,2,3,4};
	dbint arr2[1] = {1};
	dbint arr3[2] ={1,2};
	char temp1[50], temp2[50];
	int k, ll;	
	

	printJTable = 0;

	fp = fopen("medium","r");
	init_db(128);

	fgets(temp1,50,fp);
	
	while(temp1[0] != 'R')
	{
		for(i = 0 ; i < 50 ; i++)
			if(temp1[i] == '\n')
				temp1[i] = '\0';
		k = open_table(temp1,2);
		fgets(temp1, 50, fp);
	}

	while(fgets(temp1,50,fp))
	{
		k = strlen(temp1);
		temp1[k - 1] = '\0';
		fgets(temp2,50,fp);	
		k = atoi(temp2);
		ll = join(temp1);
		if(k == ll)
			printf("true!! value : %d = %d\n",k, ll);
		else
			printf("false!! value : %d != %d\n",k, ll);
	}

	shutdown_db();
	return 0;
}
