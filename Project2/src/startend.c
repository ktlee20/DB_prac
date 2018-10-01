#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "defines.h"
#include "globals.h"
#include "startend.h"

void dbstart()
{
	Page temphead;
	default_file = NULL;
	totalp = 0;
	currentp = 0;
	default_file = fopen("default.db","r+b");
	if(default_file == NULL)
	{
		default_file = fopen("default.db","w+b");
		file_alloc_page();
		temphead.headerp.freep_offset = 0;
		temphead.headerp.rootp_offset = 0;	
		temphead.headerp.numOfPage = 1;
		fwrite(&temphead,PAGESIZE,1,default_file);
		currentp = 0;
		return;
	}
	currentp = 0;

	return;
}

void headerInit()
{
	fseek(default_file,0,SEEK_END);
	fread(&Header,PAGESIZE,1,default_file);
	totalp = Header.headerp.numOfPage;
}

void dbend()
{
	fclose(default_file);
}
