#ifndef __JOIN_H_
#define __JOIN_H_

#include "defines.h"

PNode * parse(char * input);
void * tt_JTqsort(void * data);
void* t_JTqsort(void * data,int table_id);
void JTqsort(dbint** jt, int col,int length);
void cusFree(JTable * jt);
JTable * joining(PNode * left, PNode * right, JTable * jtleft, JTable * jtright);
JTable * join_table(PNode * tree);
dbint join(char * str);

#endif
