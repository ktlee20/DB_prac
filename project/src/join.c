#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "defines.h"
#include "globals.h"
#include "Optimizer.h"
#include "join.h"

pthread_t thread_t[MAXJOINNUM + 1][MAXVALNUM + 1];
int printJTable = FALSE;

PNode * parse(const char * input)
{
	int i = 0;
	int queryNum = 1;
	int slen = strlen(input);
	int arr[10];	
	char str[256];	
	char **query;
	PNode * parseTree;	

	query = (char**)malloc(sizeof(char*) * 10);

	strcpy(str,input);
	while(str[i] == ' ')
		i++;

	arr[0] = i;

	for( ; i < slen ; i++)
	{
		if(str[i] == '&')
		{
			query[queryNum - 1] = (char*)malloc(sizeof(char) * 20);
			arr[queryNum] = ++i;
			str[i - 1] = '\0';
			strcpy(query[queryNum - 1], &str[arr[queryNum - 1]]);
			queryNum++;
		}	
	}

	query[queryNum - 1] = (char*)malloc(sizeof(char) * 20);
	strcpy(query[queryNum - 1], &str[arr[queryNum - 1]]); 

	parseTree = sort_selectivity(query, queryNum);
	
	for(i = 0 ; i < queryNum ; i++)
		free(query[i]);

	free(query);

	return parseTree; 
}

void* tt_JTqsort(void * data)
{
	JTqsort(((T0*)data)->jt, ((T0*)data)->col, ((T0*)data)->length);
	return NULL;
}

void* t_JTqsort(void * data,int table_id)
{
	T0 *ttemp = (T0*)data;	
	T0 t1,t2,t3,t4;
	dbint ** jt = ttemp->jt, *pivot, *temp;
	int col = ttemp->col, length = ttemp->length;
	int tmid,mid,mid2,mid3,i,j;
	int status;
	int tt1,tt2,tt3,tt4,cc1,cc2,cc3,cc4;

	if(length <= 50)
	{
		while(thread_t[table_id][col] != -1)
		{
			col++;	
			if(col == (MAXVALNUM + 1))
			{
				col = 0; 
				table_id++;
			}
			if(table_id == (MAXJOINNUM + 1))
				table_id = 0;
		}
		pthread_create(&thread_t[table_id][col], NULL, tt_JTqsort, (void*)ttemp);
		pthread_join(thread_t[table_id][col], (void**)&status);			
		thread_t[table_id][col] = -1;
		return NULL;
	}

	mid = length / 2;
	if(length <= 100)
	{
		JTqsort(&jt[mid - 10], ttemp->col, 20);
		i = 0;
		
		pivot = jt[mid];	
		jt[mid] = jt[0];
		jt[0] = pivot;
		
		for(j = 1 ; j < length ; j++)
			if(pivot[ttemp->col] >= jt[j][ttemp->col])		
			{
				i++;
				if(i == j)
					continue;
				
				temp = jt[i];
				jt[i] = jt[j];
				jt[j] = temp;
			}

		jt[0] = jt[i];
		jt[i] = pivot; 
		t1.jt = &jt[0];
		t1.col = ttemp->col;
		t1.length = i;	
		t2.jt = &jt[i+1];
		t2.col = ttemp->col;
		t2.length = (length - i - 1);

		while(thread_t[table_id][col] != -1)
		{
			col++;	
			if(col == (MAXVALNUM + 1))
			{
				col = 0; 
				table_id++;
			}
			if(table_id == (MAXTABLE + 1))
				table_id = 0;
		}
		tt1 = table_id, cc1 = col;
		pthread_create(&thread_t[tt1][cc1], NULL, tt_JTqsort, (void*)&t1);

		while(thread_t[table_id][col] != -1)
		{
			col++;
			if(col == (MAXVALNUM + 1))
			{
				col = 0;
				table_id++;
			}
			if(table_id == (MAXTABLE + 1))
				table_id++;
		}
		tt2 = table_id, cc2 = col;
		pthread_create(&thread_t[tt2][cc2], NULL, tt_JTqsort, (void*)&t2);

		pthread_join(thread_t[tt1][cc1], (void**)&status);			
		pthread_join(thread_t[tt2][cc2], (void**)&status);			
		thread_t[tt1][cc1] = -1;
		thread_t[tt2][cc2] = -1;

		return NULL;
	}		
	
	JTqsort(&jt[mid - 25], ttemp->col, 50);

	pivot = jt[mid];
	jt[mid] = jt[0];
	jt[0] = pivot;
	i = 0;

	for(j = 1 ; j < length ; j++)
		if(pivot[ttemp->col] >= jt[j][ttemp->col])
		{
			i++;
			if(i == j)
				continue;
			
			temp = jt[i];
			jt[i] = jt[j];
			jt[j] = temp;
		}

	jt[0] = jt[i];
	jt[i] = pivot;

	mid2 = i/2;	
	mid3 = i + ((length - i) / 2);
	tmid = i;
	
	pivot = jt[mid2];
	jt[mid2] = jt[0];
	jt[0] = pivot;	
	i = 0;

	for(j = 1 ; j < tmid ; j++)
		if(pivot[ttemp->col] >= jt[j][ttemp->col])
		{
			i++;
			if(i == j)
				continue;
			
			temp = jt[i];
			jt[i] = jt[j];
			jt[j] = temp;
		}
	
	jt[0] = jt[i];
	jt[i] = pivot;
	
	t1.jt = &jt[0];
	t1.length = i;
	t1.col = ttemp->col;	
	t2.jt = &jt[i + 1];
	t2.col = ttemp->col;
	t2.length = tmid - i - 1;
	
	while(thread_t[table_id][col] != -1)
	{
		col++;
		if(col == (MAXVALNUM + 1))
		{
			col = 0;
			table_id++;
		}
		if(table_id == (MAXTABLE + 1))
			table_id = 0;
	}	
	tt1 = table_id, cc1 = col;
	thread_t[tt1][cc1] = 0;
	pthread_create(&thread_t[tt1][cc1], NULL, tt_JTqsort, (void*)&t1);	

	while(thread_t[table_id][col] != -1)
	{
		col++;
		if(col == (MAXVALNUM + 1))
		{
			col = 0;
			table_id++;
		}
		if(table_id == (MAXTABLE + 1))
			table_id = 0;
	}
	tt2 = table_id, cc2 = col;
	thread_t[tt2][cc2] = 0;
	pthread_create(&thread_t[tt2][cc2], NULL, tt_JTqsort, (void*)&t2);
	
	pivot = jt[mid3];
	jt[mid3] = jt[tmid];
	jt[tmid] = pivot;
	i = tmid;

	for(j = tmid + 1 ; j < length ; j++)
		if(pivot[ttemp->col] >= jt[j][ttemp->col])
		{
			i++;
			if(i == j)
				continue;
			
			temp = jt[i];
			jt[i] = jt[j];
			jt[j] = temp;
		}
	
	jt[tmid] = jt[i];
	jt[i] = pivot;
	
	t3.jt = &jt[tmid];
	t3.length = i - tmid;
	t3.col = ttemp->col;	
	t4.jt = &jt[i + 1];
	t4.col = ttemp->col;
	t4.length = length - i - 1;
	
	while(thread_t[table_id][col] != -1)
	{
		col++;
		if(col == (MAXVALNUM + 1))
		{
			col = 0;
			table_id++;
		}
		if(table_id == (MAXTABLE + 1))
			table_id++;
	}
	tt3 = table_id, cc3 = col;
	thread_t[tt3][cc3] = 0;
	pthread_create(&thread_t[tt3][cc3], NULL, tt_JTqsort, (void*)&t3);
	
	while(thread_t[table_id][col] != -1)
	{
		col++;
		if(col == (MAXVALNUM + 1))
		{
			col = 0;
			table_id++;
		}
		if(table_id == (MAXTABLE + 1))
			table_id = 0;
	}	
	tt4 = table_id, cc4 = col;
	thread_t[tt4][cc4] = 0;
	pthread_create(&thread_t[tt4][cc4], NULL, tt_JTqsort, (void*)&t4);

	pthread_join(thread_t[tt1][cc1], (void**)&status);
	pthread_join(thread_t[tt2][cc2], (void**)&status);
	pthread_join(thread_t[tt3][cc3], (void**)&status);
	pthread_join(thread_t[tt4][cc4], (void**)&status);
	
	thread_t[tt1][cc1] = thread_t[tt2][cc2] = thread_t[tt3][cc3] = thread_t[tt4][cc4] = -1;
	return NULL;
	//qsort 한꺼번에 처음에 진행 join을 시작하면서 받는다.
}

void JTqsort(dbint** jt, int col,int length)
{
	int i = 0,j,mid;
	dbint * temp, *pivot;	
	
	if(length <= 1)
		return;

	if(length == 2)
		if(jt[0][col] > jt[1][col])
		{
			temp = jt[0];
			jt[0] = jt[1];
			jt[1] = temp;
			return;
		}
	mid = length/2;	
	pivot = jt[mid];
	jt[mid] = jt[0];
	jt[0] = pivot;

	for(j = 1 ; j < length ; j++)
		if(pivot[col] >= jt[j][col])
		{
			i++;
			if(i == j)
				continue;

			temp = jt[i];
			jt[i] = jt[j];
			jt[j] = temp;
		}
	
	jt[0] = jt[i];
	jt[i] = pivot;

	JTqsort(&jt[0], col, i);
	if(i < length - 1)
		JTqsort(&jt[i + 1], col ,length - i - 1);
}

JTable * joining(PNode * left, PNode * right, JTable * jtleft, JTable * jtright)
{
	int i, j, l ,r, num, mark;
	int lcol , rcol;
	PNode * parent = left->parent;
	JTable * newJtleft;	
	dbint ** tempTable = (dbint**)malloc(sizeof(dbint*));
	int *rtid = jtright->tid, *ltid = jtleft->tid;
	int ntid[MAXTABLE];
	T0 t_args1, t_args2;

	newJtleft = (JTable*)malloc(sizeof(JTable));
	newJtleft->joinedNum = jtleft->joinedNum + jtright->joinedNum;
	newJtleft->numOfData = 0;

	
	for(i = 0 ; i < jtleft->joinedNum ; i++)
	{
		newJtleft->tid[i] = jtleft->tid[i];	
		newJtleft->colSize[i] = jtleft->colSize[i];
		if(left->table_id == jtleft->tid[i])
			lcol = (i == 0) ? (left->col - 1) : (left->col + jtleft->colSize[i - 1] - 1);
	}	
	for(j = 0 ; j < jtright->joinedNum ; j++)
	{
		newJtleft->tid[i + j] = jtright->tid[j];
		newJtleft->colSize[i + j] = jtright->colSize[j] + newJtleft->colSize[i + j - 1];
		if(right->table_id == jtright->tid[j])
			rcol = (j == 0) ? (right->col - 1) : (right->col + jtright->colSize[i - 1] - 1);
	}

	tempTable[0] = (dbint*)malloc(sizeof(dbint) * newJtleft->colSize[newJtleft->joinedNum - 1]);
	
	t_args1.jt = jtleft->iArr;
	t_args1.col = lcol;
	t_args1.length = jtleft->numOfData;	

	t_args2.jt = jtright->iArr;
	t_args2.col = rcol;
	t_args2.length = jtright->numOfData;

	t_JTqsort(&t_args1,left->table_id);
	t_JTqsort(&t_args2,right->table_id);

	l = 0 , r = 0;
	
	if(jtleft->numOfData <= 0 || jtright->numOfData <= 0)
		return NULL;
	
	while(l < jtleft->numOfData || r < jtright->numOfData)
	{
		while(jtleft->iArr[l][lcol] < jtright->iArr[r][rcol] || jtleft->iArr[l][lcol] > jtright->iArr[r][rcol])
		{
			if(jtleft->iArr[l][lcol] < jtright->iArr[r][rcol])
			{
				l++;
				if(l >= jtleft->numOfData)
					break;
				else
					continue;
			}
			if(jtleft->iArr[l][lcol] > jtright->iArr[r][rcol])
			{
				r++;
				if(r >= jtright->numOfData)
					break;
				else
					continue;
			}
		}

		if(l >= jtleft->numOfData && r >= jtright->numOfData)
			break;
		if(l >= jtleft->numOfData)
		{
			if(jtleft->iArr[jtleft->numOfData - 1][lcol] <= jtright->iArr[r][rcol])
				break;
			else
				l = (jtleft->numOfData - 1);
		}
		if(r >= jtright->numOfData)
		{		
			if(jtright->iArr[jtright->numOfData - 1][rcol] <= jtleft->iArr[l][lcol])
				break;
			else
				r = (jtright->numOfData - 1);
		}

		mark = r;
		while(jtleft->iArr[l][lcol] == jtright->iArr[r][rcol])
		{
			while(jtleft->iArr[l][lcol] == jtright->iArr[r][rcol])
			{
				if(newJtleft->numOfData == 0)
				{
					for(i = 0 ; i < jtleft->colSize[jtleft->joinedNum - 1] ; i++)
						tempTable[0][i] = jtleft->iArr[l][i];
					for(j = 0 ; j < jtright->colSize[jtright->joinedNum - 1] ; j++)
						tempTable[0][i + j] = jtright->iArr[r][j];
					newJtleft->numOfData = 1;
				}
				else
				{
					newJtleft->numOfData++;
					tempTable = (dbint**)realloc(tempTable,sizeof(dbint*) * newJtleft->numOfData);
					tempTable[newJtleft->numOfData - 1] = (dbint*)malloc(sizeof(dbint) * newJtleft->colSize[newJtleft->joinedNum - 1]);
					for(i = 0 ; i < jtleft->colSize[jtleft->joinedNum - 1] ; i++)
						tempTable[newJtleft->numOfData - 1][i] = jtleft->iArr[l][i];
					for(j = 0 ; j < jtright->colSize[jtright->joinedNum - 1] ; j++)
						tempTable[newJtleft->numOfData - 1][i + j] = jtright->iArr[r][j];
				}
				r++;
				if(r >= jtright->numOfData)
				{
					break;
				}
			}	
			r = mark;
			l++;
			if(l >= jtleft->numOfData)
			{
				break;
			}
		}

		if(l >= jtleft->numOfData && r >= jtright->numOfData)
			break;
		if(l >= jtleft->numOfData)
		{
			if(jtleft->iArr[jtleft->numOfData - 1][lcol] <= jtright->iArr[r][rcol])
				break;
			else
				l--;
		}
		if(r >= jtright->numOfData)
		{		
			if(jtright->iArr[jtright->numOfData - 1][rcol] <= jtleft->iArr[l][lcol])
				break;
			else
				r--;
		}
	}
	newJtleft->arrSize = newJtleft->numOfData;
	newJtleft->iArr = tempTable;
	
	return newJtleft;
}

void cusFree(JTable * jt)
{
	int i;
	
	for(i = 0 ; i < jt->numOfData ; i++)
	{
		free(jt->iArr[i]);
	}
	free(jt->iArr);
	free(jt);
}

JTable * join_table(PNode * tree)
{
	PNode * temp = tree, *left, *right;
	JTable *jtleft,*jtright,*jttemp = NULL;
	int i,j,t;
	int oneQuery = 0;	

	if(temp->left->left == NULL)
		oneQuery = 1;

	while(temp->left != NULL)
		temp = temp->left;	

	temp = temp->parent;
	left = temp->left;
	right = temp->right;
	jtleft = memory_key[left->table_id];
	t = left->table_id;
	jtright = memory_key[right->table_id];

	while(temp->table_id != INITTREE)
	{
		jttemp = joining(left,right,jtleft, jtright);	
		if(jtleft != memory_key[t])
			cusFree(jtleft);
		if(jttemp == NULL)
			return NULL;
		jtleft = jttemp;
		temp = temp->parent;
		left = temp->left;
		right = temp->right;
		jtright = memory_key[right->table_id];
	}

	jttemp = joining(left,right, jtleft, jtright);
	
	if(oneQuery != 1)
		cusFree(jtleft);
	jtleft = jttemp;

	if(printJTable)
	{
		for(j = 0 ; j < jtleft->numOfData ; j++)
		{
			for(i = 0 ; i < jtleft->colSize[jtleft->joinedNum - 1] ; i++)
				printf("%ld ",jtleft->iArr[j][i]);
			printf("\n");
		}	
	}	
	return jtleft;
}

////////////qsort 미리 이전에 수행하도록 당긴다.


dbint join(char const* str)
{
	int i,j;
	PNode * pTree = parse(str);
	dbint key = 0;
	JTable * jttemp;

	if(pTree == NULL)
		return 0;

	jttemp = join_table(pTree);
	
	if(jttemp == NULL)
		return 0;

	for(j = 0 ; j < jttemp->numOfData ; j++)
	{
		key += jttemp->iArr[j][0];
		for(i = 0 ; i < jttemp->joinedNum - 1 ; i++)
			key += jttemp->iArr[j][jttemp->colSize[i]];
	}

	free(jttemp);
	//printf("%ld\n",key);
	
	return key;
}
