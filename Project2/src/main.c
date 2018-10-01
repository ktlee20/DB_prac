#include <stdio.h>
#include "defines.h"
#include "globals.h"
#include "startend.h"
#include "page.h"

//중간에 코드를 통째로 날렸어서.. 깃에 백업을 하겠습니다.ㅠㅠ

FILE * default_file;
pagenum_t totalp;
pagenum_t currentp;
Page Header;
Page rootnodep;

int main(int argc, char * argv)
{
	dbstart();
	headerInit();


	dbend();
	return 0;
}
