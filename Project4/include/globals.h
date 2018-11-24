#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include "defines.h"

extern FILE ** default_file;
extern int * default_fd;
extern Page ** header;
extern Page ** rootpage;
extern pagenum_t * totalp;
extern pagenum_t * currentp;

//BufferManager.c
extern Bufstrt * buffer[MAXTABLE];
extern Bufstrt * bufferlast[MAXTABLE];
extern int bufsize;
extern int curPos[MAXTABLE];
extern int numOfCol[MAXTABLE];

//Optimizer.c
extern OptInfo * oinfo[MAXTABLE];
extern JTable * memory_key[MAXTABLE];

#endif
