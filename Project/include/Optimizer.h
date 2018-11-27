#ifndef __OPTIMIZER_H__
#define __OPTIMIZER_H__

#include "defines.h"

int process_records(int table_id, Records * record);
int read_table(int table_id, Page * c);
int make_stat(int table_id);
int inmemory_scanner(int table_id, Page * c);
PNode * makeNode(int t, int c);
void QInit(Queue * q);
int QIsEmpty(Queue * q);
void Enqueue(Queue * q, QData data);
QData pip(Queue * q);
QData Dequeue(Queue * q);
int arrCheck(int * num, int ta);
PNode * make_parsetree(Queue * qi, int *t1, int *t2, int *c1, int *c2, int queryNum);
int tCheck(PNode * temp, int table_id, int col);
void addNeed(PNode * parent, PNode * temp);
PNode * make_need_info(PNode * tHead);
double selectivity(int t1, int t2, int c1, int c2);
PNode * sort_selectivity(char ** query, int queryNum);

#endif
