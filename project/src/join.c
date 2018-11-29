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

PNode * parse(char * input)
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
	
	JTqsort(jtleft->iArr, lcol, jtleft->numOfData);	
	JTqsort(jtright->iArr, rcol, jtright->numOfData);
	
	l = 0 , r = 0;
	
	while(l < jtleft->numOfData || r < jtright->numOfData)
	{
		if(jtleft->numOfData <= 0 || jtright->numOfData <= 0)
			return NULL;		
	
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
			if(jtleft->iArr[jtleft->numOfData - 1][lcol] <= jtright->iArr[r][rcol]) // numOfData == 0일 때 예외처리!!
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

dbint join(char * str)
{
	int i,j;
	PNode * pTree = parse(str);
	dbint key = 0;
	JTable * jttemp;

	if(pTree == NULL)
	{
		printf("%ld\n",key);
		return 0;
	}
	jttemp = join_table(pTree);
	
	if(jttemp == NULL)
	{
		//printf("%ld\n",key);
		return 0;
	}
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
