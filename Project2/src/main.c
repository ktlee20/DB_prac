#include <stdio.h>
#include "defines.h"
#include "globals.h"
#include "startend.h"
#include "page.h"

FILE * default_file;
pagenum_t totalp;
pagenum_t currentp;
Page Header;
Page * rootnodep;

int main(int argc, char * argv)
{
	Page Header;

	dbstart();
	headerInit();


	dbend();
	return 0;
}
