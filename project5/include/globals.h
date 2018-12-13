#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include "defines.h"
#include <stdio.h>
#include <pthread.h>

extern FILE ** default_file;
extern int * default_fd;
extern Page ** header;
extern Page ** rootpage;
extern pagenum_t * totalp;
extern pagenum_t * currentp;

//BufferManager.c
extern Buffer_Control_Block bufctrl[MAXTABLE+1];
extern int numOfCol[MAXTABLE+1];

//Optimizer.c
extern OptInfo * oinfo[MAXTABLE+1];
extern JTable * memory_key[MAXTABLE+1];

//join.c
extern int printJTable;
extern pthread_t thread_t[MAXJOINNUM + 1][MAXVALNUM + 1];

//LockManager.cpp
extern int txnNumber;
extern LockTable locktable[MAXTABLE + 1];
extern TXNList txnlist;
extern uint64_t timer[MAXTABLE + 1];

#endif
