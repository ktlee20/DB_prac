#ifndef __JOIN_H_
#define __JOIN_H_

#include "defines.h"

PNode * parse(char * input);
void JTqsort(dbint ** jt, int col, int length);
JTable * joining(PNode * left, PNode * right, JTable * jtleft, JTable * jtright);
JTable * join_table(PNode * tree);
dbint join(char * str);

#endif
