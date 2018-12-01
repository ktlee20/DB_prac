#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BufferManager.h"
#include "DiskManager.h"
#include "defines.h"
#include "join.h"
#include "globals.h"
#include "Optimizer.h"

OptInfo * oinfo[MAXTABLE+1];
JTable * memory_key[MAXTABLE+1];

int process_records(int table_id, Records* record)
{
	int i;
	int col_num = numOfCol[table_id];	


	if(oinfo[table_id]->num == FAIL)
	{
		oinfo[table_id]->num = 1;
		oinfo[table_id]->mm[1].min = oinfo[table_id]->mm[1].max = record->key;
		for(i = 2 ; i <= col_num  ; i++)
			oinfo[table_id]->mm[i].min = oinfo[table_id]->mm[i].max = record->values[i-2];

		return 0;
	}	
	
	oinfo[table_id]->num++;
	if(record->key < oinfo[table_id]->mm[1].min)
		oinfo[table_id]->mm[1].min = record->key;
	else if(record->key > oinfo[table_id]->mm[1].max)
		oinfo[table_id]->mm[1].max = record->key;

	for(i = 2 ; i <= col_num ; i++)
	{
		if(oinfo[table_id]->mm[i].min > record->values[i - 2])
			oinfo[table_id]->mm[i].min = record->values[i - 2];
		else if(oinfo[table_id]->mm[i].max < record->values[i - 2])
			oinfo[table_id]->mm[i].max = record->values[i - 2];
	}
	
	return 0;
}

int read_table(int table_id, Page * c)
{
	int i = 0;
	dbint offset;
	Page * temp = (Page*)malloc(sizeof(Page));	

	if(c->leafp.pheader.is_leaf)
	{
		for(i = 0 ; i < c->leafp.pheader.numOfKeys ; i++)
			process_records(table_id, &c->leafp.records[i]);				
		inmemory_scanner(table_id,c);
		free(temp);
		return 1;
	}

	offset = c->internalp.pheader.other_page_offset;
	if(offset != 0)
	{
		buf_read_page(offset / PAGESIZE, temp, table_id);
		read_table(table_id, temp);
	}
	for(i = 0 ; i < c->internalp.pheader.numOfKeys ; i++)
	{
		offset = c->internalp.entities[i].offset;	
		if(offset != 0)
		{
			buf_read_page(offset/PAGESIZE, temp, table_id);
			read_table(table_id, temp);
		}
	}

	free(temp);
	return 1;
}

int make_stat(int table_id)
{
	int i,j;
	
	oinfo[table_id] = (OptInfo*)malloc(sizeof(OptInfo));
	oinfo[table_id]->num = FAIL;

	for(i = 0 ; i < MAXVALNUM ; i++)
	{
		oinfo[table_id]->mm[i].min = FAIL;
		oinfo[table_id]->mm[i].max = FAIL;
	}	
	read_table(table_id, rootpage[table_id]);
	return 0;
}

int inmemory_scanner(int table_id, Page * c)
{
	int i,j,temp;

	if(memory_key[table_id] == NULL)
	{
		memory_key[table_id] = (JTable*)malloc(sizeof(JTable));
		memory_key[table_id]->numOfData = 0;
		memory_key[table_id]->arrSize = c->leafp.pheader.numOfKeys;
		memory_key[table_id]->iArr = (dbint**)malloc(sizeof(dbint*) * c->leafp.pheader.numOfKeys );
		memory_key[table_id]->joinedNum = 1;
		memory_key[table_id]->tid[0] = table_id;
		memory_key[table_id]->colSize[0] = numOfCol[table_id];
		for(i = 1 ; i < MAXTABLE ; i++)
			memory_key[table_id]->tid[i] = memory_key[table_id]->colSize[i] = 0;		

	}
	else
	{
		memory_key[table_id]->arrSize += c->leafp.pheader.numOfKeys;
		memory_key[table_id]->iArr = (dbint**)realloc(memory_key[table_id]->iArr, memory_key[table_id]->arrSize * sizeof(dbint));	
	}

	for(i = 0 ; i < c->leafp.pheader.numOfKeys ; i++)
	{
		temp = i + memory_key[table_id]->numOfData;
		memory_key[table_id]->iArr[temp] = (dbint*)malloc(sizeof(dbint) * numOfCol[table_id]);	
		memory_key[table_id]->iArr[temp][0] = c->leafp.records[i].key;
		for(j = 0 ; j < (numOfCol[table_id] - 1) ; j++)
			memory_key[table_id]->iArr[temp][j + 1] = c->leafp.records[i].values[j];
	}	
	memory_key[table_id]->numOfData += c->leafp.pheader.numOfKeys;

	return 0;
}

PNode * makeNode(int t, int c)
{
	int i;
	PNode * temp = (PNode*)malloc(sizeof(PNode));

	temp->table_id = t;
	temp->col = c;
	temp->right = NULL;
	temp->left = NULL;
	temp->parent = NULL;
	temp->sorted = 0;
	temp->qsnum = 0;

	for(i = 0 ; i < QSTHREAD ; i++)
		temp->tt[i] = temp->cc[i] = 0;

	return temp;
}

void QInit(Queue * q)
{
	q->numOfData = 0;
	q->front = NULL;
	q->rear = NULL;
}

int QIsEmpty(Queue * q)
{
	if(q->numOfData == 0)
		return 1;
	
	return 0;
}

void Enqueue(Queue * q, QData data)
{
	q->numOfData++;
	if(q->front == NULL)
	{
		q->front = q->rear = (QNode*)malloc(sizeof(QNode));
		q->front->next = NULL;
	}	
	else
	{
		q->front->prev = (QNode*)malloc(sizeof(QNode));	
		q->front->prev->next = q->front;
		q->front = q->front->prev;
	}
	q->front->data = data;
	q->front->prev = NULL;
}

QData pip(Queue * q)
{
	return q->rear->data;
}

QData Dequeue( Queue * q)
{
	QData tmp = pip(q);
	QNode * temp = q->rear;

	if(QIsEmpty(q))
		return -1;

	q->rear = temp->prev;

	if(q->rear != NULL)
		q->rear->next = NULL;
	else
		q->front = NULL;

	free(temp);
	q->numOfData--;

	return tmp;
}

double selectivity(int t1,int t2, int c1, int c2)
{
	int min1, min2, max1,max2,num1,num2,factor;
	int range1, range2;
	double speculation1, speculation2;

	min1 = oinfo[t1]->mm[c1].min;
	min2 = oinfo[t2]->mm[c2].min;
	max1 = oinfo[t1]->mm[c1].max;
	max2 = oinfo[t2]->mm[c2].max;
	num1 = oinfo[t1]->num;
	num2 = oinfo[t2]->num;
	range1 = (max1 == min1) ? 1 : max1 - min1;	
	range2 = (max2 == min2) ? 1 : max2 - min2;

	if(min1 < min2)
	{
		if(max1 < max2)
		{
			if(max1 < min2)
				factor = 0;	
			else if(max1 == min2)
				factor = 1;
			else
				factor = max1 - min2 + 1;
		}
		else
		{
			factor = range2;
		}
	}	
	else if(min1 == min2)
	{
		if(max1 < max2)
		{
			factor = range1;
		}
		else
		{
			factor = range2;
		}
	}
	else
	{
		if(max2 < max1)
		{
			if(max2 < min1)
				factor = 0;
			else if(max2 == min1)
				factor = 1;
			else
				factor = max2 - min1 + 1;
		}
		else
		{
			factor = range1;
		}
	}
	speculation1 = num1 * ((double)factor / range1);
	speculation2 = num2 * ((double)factor / range2);

	return speculation1 + speculation2;
}

int arrCheck(int * num, int ta)
{
	int i = 0;

	while(num[i] != -1)
	{
		if(ta == num[i])
			return i;
		i++;
	}
	
	return -1;
}

PNode * make_parsetree(Queue * q,int *t1, int *t2, int * c1, int * c2, int queryNum)
{
	PNode * rtemp, *ltemp, *ptemp = NULL;
	int * num = (int*)malloc(sizeof(int) * queryNum * 2);
	int j,ta,tb,ca,cb,i = 0;	
	
	for(j = 0 ; j < (queryNum * 2) ; j++)
		num[j] = -1;

	while(!QIsEmpty(q))
	{
		j = Dequeue(q);
		
		ta = t1[j], tb = t2[j], ca = c1[j], cb = c2[j];
		if(ptemp == NULL)
		{
			rtemp = makeNode(ta,ca);	
			ltemp = makeNode(tb,cb);
			ptemp = makeNode(INITTREE, INITTREE);
			ptemp->right = rtemp;
			ptemp->left = ltemp;
			rtemp->parent = ptemp;
			ltemp->parent = ptemp; 
			num[i++] = ta;
			num[i++] = tb;
			continue;
		}
		
		if(arrCheck(num, ta) != -1)
		{
			ltemp = ptemp;
			ltemp->table_id = ta;
			ltemp->col = ca;	
			rtemp = makeNode(tb,cb);	
			ptemp = makeNode(INITTREE, INITTREE);
			ptemp->left = ltemp;
			ptemp->right = rtemp;
			rtemp->parent = ptemp;
			ltemp->parent = ptemp;
			num[i++] = ta;
			num[i++] = tb;
		}	
		else
		{
			ltemp = ptemp;
			ltemp->table_id = tb;
			ltemp->col = cb;
			rtemp = makeNode(ta,ca);
			ptemp = makeNode(INITTREE, INITTREE);
			ptemp->left = ltemp;
			ptemp->right = rtemp;
			rtemp->parent = ptemp;
			ltemp->parent = ptemp;
			num[i++] = tb;
			num[i++] = ta;
		}
	}
	
	free(q);
	free(num);
	return ptemp;
}

PNode* sort_selectivity(char ** query, int queryNum)
{
	int i,j,b1,b2,k;
	PNode *ptemp;
	int *t1,*t2,*c1,*c2,temp,len;
	int * using;
	double * slt, stemp,stemp2;
	Queue *q, *q1, *qtemp;

	t1 = (int*)malloc(sizeof(int) * queryNum);
	t2 = (int*)malloc(sizeof(int) * queryNum);
	c1 = (int*)malloc(sizeof(int) * queryNum);
	c2 = (int*)malloc(sizeof(int) * queryNum);
	slt = (double*)malloc(sizeof(double) * queryNum);
	using = (int*)malloc(sizeof(int) * (MAXTABLE+1));
	q = (Queue*)malloc(sizeof(Queue));
	q1 = (Queue*)malloc(sizeof(Queue));

	for(i = 0 ; i < (MAXTABLE + 1) ; i++)
		using[i] = 0;

	QInit(q);
	QInit(q1);

	for(i = 0 ; i < queryNum ; i++)
	{
		len = strlen(query[i]);
		for(j = 0 ; j < len ; j++)
		{
			if(query[i][j] == '.')
			{
				query[i][j] = '\0';
				t1[i] = atoi(query[i]);
				k = j + 1;
				break;
			}
		}

		for(; j < len ; j++)
		{
			if(query[i][j] == '=')
			{
				query[i][j] = '\0';
				c1[i] = atoi(&query[i][k]);
				k = j + 1;
				break;
			}
		}

		for(; j < len ; j++)
		{
			if(query[i][j] == '.')
			{
				query[i][j] = '\0';
				t2[i] = atoi(&query[i][k]);
				k = j + 1;
				break;
			}
		}
		c2[i] = atoi(&query[i][k]);
		slt[i] = selectivity(t1[i],t2[i],c1[i],c2[i]);
	}
		
	for(i = queryNum ; i > 0 ; i--)
		for(j = 0  ; j < i - 1; j++)
		{
			if(slt[j] > slt[j + 1])
			{
				stemp2 = slt[j + 1];
				slt[j + 1] = slt[j];
				slt[j] = stemp2;

				temp = t1[j + 1];
				t1[j + 1] = t1[j];
				t1[j] = temp;

				temp = t2[j + 1];
				t2[j + 1] = t2[j];
				t2[j] = temp;
	
				temp = c1[j + 1];
				c1[j + 1] = c1[j];
				c1[j] = temp;

				temp = c2[j + 1];
				c2[j + 1] = c2[j];
				c2[j] = temp;
				
			}		
		} 

	j = 0;

	if(slt[0] == 0)
		return NULL;	
	
	Enqueue(q,j); 	

	using[j] = 1;
	
	while(q->numOfData != queryNum)
	{
		stemp = 18446744073709552000.000;
		k = -1;
		while(!QIsEmpty(q))
		{
			j = Dequeue(q);			
			Enqueue(q1,j);
			b1 = t1[j], b2 = t2[j];	
			for(i = 0 ; i < queryNum ; i++)
			{
				if((using[i] != 1) && (t1[i] == b1 || t2[i] == b1 || t1[i] == b2 || t2[i] == b2) && stemp > slt[i])
				{
					stemp = slt[i];
					k = i;	
				}
			}				
		}
		if(k == -1)
			return NULL;

		Enqueue(q1,k);
		using[k] = 1;
		free(q);
		q = q1;
		q1 = (Queue*)malloc(sizeof(Queue));	
		QInit(q1);
	}
	free(q1);
	ptemp = make_parsetree(q, t1,t2,c1,c2,queryNum);	
	
	free(t1);
	free(t2);
	free(c1);
	free(c2);
	free(slt);
	free(using);

	return ptemp;
}
