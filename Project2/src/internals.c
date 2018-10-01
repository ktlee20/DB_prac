#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "globals.h"
#include "page.h"

void readInternal()
{
	rootnodep = (Page*)malloc(sizeof(Page));	
	file_read_page(Header.headerp.rootp_offset, &rootnodep);	
}
