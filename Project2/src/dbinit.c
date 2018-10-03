#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "globals.h"
#include "FileIndexManager.h"

void dbend()
{
	if(header != NULL);
		free(header);
	if(rootpage != NULL);
		free(rootpage);
	if(default_file != NULL)
		free(default_file);
}

