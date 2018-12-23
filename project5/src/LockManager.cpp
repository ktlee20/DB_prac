#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <iostream>
#include "defines.h"
#include "globals.h"
#include "BufferManager.h"
#include "DiskManager.h"
#include "LockManager.h"

using namespace std;

int txnNumber = 0;
LockTable locktable[MAXTABLE + 1];
TXNList txnlist;
uint64_t timer[MAXTABLE + 1];

txn_t * makeTxn(int tid)
{
	txn_t * retTxn = (txn_t*)malloc(sizeof(txn_t));

	retTxn->tid = tid;
	retTxn->mode = EMBRYO;
	retTxn->txn_locks = NULL;
	retTxn->wait_locks = NULL;
	retTxn->lognum = 0;
	retTxn->logs = NULL;

	return retTxn;
}

lock_t * makeLock(int table_id, txn_t * txn, dbint page_id, uint64_t timestamp, LMODE mode, Bufstrt * buf)
{
	lock_t * retLock = (lock_t*)malloc(sizeof(lock_t));	
	LNode * node = (LNode*)malloc(sizeof(LNode)), *temp;

	retLock->table_id = table_id;
	retLock->txn = txn;
	retLock->page_id = page_id;
	retLock->timestamp = timestamp;
	retLock->mode = mode;
	retLock->cond = PTHREAD_COND_INITIALIZER;	
	retLock->buffer = buf;

	node->lock = retLock;	
	node->next = NULL;
	
	temp = txn->txn_locks;	
	
	if(temp == NULL)
	{
		txn->txn_locks = node;	
		
	}
	while(temp->next != NULL)
		temp = temp->next;
	
	temp->next = node;

	return retLock;
}

DLNode * makeDLNode(lock_t * lock)
{
	DLNode * retDLNode = (DLNode*)malloc(sizeof(DLNode));
	retDLNode->isvisit = 0;
	retDLNode->lock = lock;
	retDLNode->next = NULL;
	retDLNode->prev = NULL;

	return retDLNode;
}

void TXN_LInsert(txn_t * txn)
{
	TXNLNode * temp, *newNode;

	pthread_mutex_lock(&txnlist.txn_list_mutex);
	if(txnlist.head == NULL)
	{
		txnlist.head = txnlist.tail = (TXNLNode*)malloc(sizeof(TXNLNode));
		txnlist.head->txn = txnlist.tail->txn = txn;	
		txnlist.head->prev = txnlist.head->next = txnlist.tail->prev = txnlist.tail->next = NULL;
	}
	else
	{
		temp = txnlist.tail;
		newNode = (TXNLNode*)malloc(sizeof(TXNLNode));
		newNode->txn = txn;

		if(temp->txn->tid < txn->tid)
		{
			temp->next = newNode;
			newNode->prev = temp;
			txnlist.tail = newNode;
		}
		else
		{
			while(temp->prev != NULL && temp->txn->tid > txn->tid)
				temp = temp->prev;
			
			if(temp->txn->tid < txn->tid)
			{
				newNode->next = temp->next;
				temp->next = newNode;
				newNode->prev = temp;
			}
			else
			{
				newNode->next = temp;	
				temp->prev = newNode;
				newNode->prev = NULL;
				txnlist.head = newNode;
			}
		}
	}
	pthread_mutex_unlock(&txnlist.txn_list_mutex);
}

txn_t * TXN_LFind(int tid)
{
	TXNLNode * temp;
	
	pthread_mutex_lock(&txnlist.txn_list_mutex);

	temp = txnlist.head;

	while(temp != NULL && temp->txn->tid != tid)
		temp = temp->next;

	pthread_mutex_unlock(&txnlist.txn_list_mutex);
	
	if(temp == NULL)
		return NULL;	

	return temp->txn;
}

int txnSize()
{
	int size;
	pthread_mutex_lock(&txnlist.txn_list_mutex);
	size = txnlist.tail->txn->tid - txnlist.head->txn->tid;
	pthread_mutex_unlock(&txnlist.txn_list_mutex);

	return size;
}

int txnHead()
{
	int head;
	pthread_mutex_lock(&txnlist.txn_list_mutex);
	head = txnlist.head->txn->tid;
	pthread_mutex_unlock(&txnlist.txn_list_mutex);

	return head;
}

int txnTail()
{
	int tail;
	pthread_mutex_lock(&txnlist.txn_list_mutex);
	tail = txnlist.tail->txn->tid;
	pthread_mutex_unlock(&txnlist.txn_list_mutex);
	
	return tail;
}

txn_t * TXN_LDelete(int tid)
{
	TXNLNode * temp, * temp2;
	txn_t * retTxn;

	pthread_mutex_lock(&txnlist.txn_list_mutex);
	
	if(txnlist.head == NULL)
	{
		retTxn = NULL;	
	}
	else
	{
		temp = txnlist.head;
		if(temp->txn->tid == tid)
		{
			txnlist.head = temp->next;
			txnlist.head->prev = NULL;
			retTxn = temp->txn;
			free(temp);
		}
		else
		{
			while(temp->next != NULL && temp->txn->tid != tid)
				temp = temp->next;
			
			if(temp->txn->tid != tid)
				retTxn = NULL;
			else
			{
				if(temp->next == NULL)
					txnlist.tail = temp->prev;
				else
					temp->next->prev = temp->prev;

				temp->prev->next = temp->next;
				retTxn = temp->txn;	
				free(temp);
			}
		}
	}
	pthread_mutex_unlock(&txnlist.txn_list_mutex);

	return retTxn;
}

lock_t * find_lock_table(dbint page_id, int tid, int table_id, LMODE mode)
{
	int hash;
	tHash * thash;
	txn_t * findTxn;
	lock_t * retlock;
	DLNode * dltemp;
		
	hash = page_id % HSIZE;

	thash = locktable[table_id].HashTable[hash];

	while(thash->page_id != page_id && thash->next != NULL)
		thash = thash->next;
	
	if(thash == NULL)
		return NULL;
	dltemp = thash->head;

	while(dltemp->next != NULL && (dltemp->lock->txn->tid != tid || dltemp->lock->mode != mode))
		dltemp = dltemp->next;
	
	if(dltemp->lock->txn->tid == tid && dltemp->lock->mode == mode)
	{
		return dltemp->lock;
	}
	else 
	{
		return NULL;
	}
}

DLNode * dl_find_lock_table(dbint page_id, int tid, int table_id, LMODE mode)
{
	int hash;
	tHash * thash;
	txn_t * findTxn;
	lock_t * lock;
	DLNode * dltemp;

	hash = page_id % HSIZE;
	
	thash = locktable[table_id].HashTable[hash];

	while(dltemp->next != NULL && (dltemp->lock->txn->tid != tid || dltemp->lock->mode != mode))
		dltemp = dltemp->next;

	if(dltemp->lock->txn->tid == tid && dltemp->lock->mode == mode)
	{
		return dltemp;
	}
	else
	{
		return NULL;
	}
}

int set_lock(txn_t * txn, dbint page_id, int tid, int table_id, uint64_t timestamp, Bufstrt * buf, LMODE mode)
{
	lock_t * newlock = makeLock(table_id, txn, page_id, timestamp, mode, buf);
	int hash = page_id % HSIZE;	
	int retVal;
	tHash * thash;
	DLNode *dltemp, *dltemp2;


	if(locktable[table_id].HashTable[hash] == NULL)
	{
		locktable[table_id].HashTable[hash] = (tHash*)malloc(sizeof(tHash));
		locktable[table_id].HashTable[hash]->page_id = page_id;
		dltemp = locktable[table_id].HashTable[hash]->head = locktable[table_id].HashTable[hash]->tail = makeDLNode(newlock);
		locktable[table_id].HashTable[hash]->next = NULL;
	}
	else
	{
		thash = locktable[table_id].HashTable[hash];
		while(thash->page_id != page_id && thash->next != NULL)
			thash = thash->next;
	
		if(thash->page_id == page_id)
		{
			dltemp = locktable[table_id].HashTable[hash]->tail->next = makeDLNode(newlock);
			locktable[table_id].HashTable[hash]->tail->next->prev = locktable[table_id].HashTable[hash]->tail;
			locktable[table_id].HashTable[hash]->tail = locktable[table_id].HashTable[hash]->tail->next;
		}
		else
		{
			thash->next = (tHash*)malloc(sizeof(tHash));
			thash->next->page_id = page_id;
			dltemp = thash->next->head = thash->next->tail = makeDLNode(newlock);
			thash->next->next = NULL;
		}
	}
	dltemp2 = locktable[table_id].HashTable[hash]->tail;
	
	if(dltemp2 == locktable[table_id].HashTable[hash]->head)
		retVal = 1;
	else
	{
		while(dltemp2->prev != NULL && dltemp2->lock->mode != EXCLUSIVE)
			dltemp2 = dltemp2->prev;

		if(dltemp2->lock->mode == EXCLUSIVE)
			retVal = 2;
		else
			retVal = 1;
	}	
	
	return retVal;
}

int wake_up_txn(DLNode * dltemp)
{
	lock_t * nlock;
	txn_t * ntxn = dltemp->lock->txn;
	LNode * locknode = ntxn->txn_locks;
	DLNode * dltemp2;

	while(locknode != NULL)
	{
		nlock = locknode->lock;	
		dltemp2 = dl_find_lock_table(nlock->page_id, nlock->txn->tid, nlock->table_id, nlock->mode);
		while(dltemp2->prev != NULL && dltemp2->lock->mode != EXCLUSIVE)
			dltemp2 = dltemp2->prev;		

		if(dltemp2->lock->mode == EXCLUSIVE)
			return 0;

		locknode = locknode->next;
	}		
	
	ntxn->wait_locks = NULL;	
	locknode = ntxn->txn_locks;
	
	pthread_mutex_lock(&nlock->lock_mutex);
	ntxn->mode = RUNNING;
	while(locknode != NULL)
	{
		nlock = locknode->lock;
		pthread_cond_signal(&nlock->cond);
		pthread_mutex_unlock(&nlock->lock_mutex);
		locknode = locknode->next;
	}

	return 1;
}

lock_t * insert_lock_table(dbint page_id, int tid, int table_id, uint64_t timestamp, LMODE mode)
{
	int hash, flags;
	tHash * thash;
	txn_t * ttxn;
	lock_t * tlock;	
	Bufstrt * buf = get_buf_node(page_id, table_id);
	buf->is_pinned++;

	pthread_mutex_lock(&locktable[table_id].ltmutex);
	
	if((ttxn = TXN_LFind(tid)) == NULL)
	{
		pthread_mutex_unlock(&locktable[table_id].ltmutex);
		return NULL;
	}
	if((tlock = find_lock_table(page_id, tid, table_id, mode)) != NULL)	
	{
		pthread_mutex_unlock(&locktable[table_id].ltmutex);
		return NULL;	
	}
	flags = set_lock(ttxn, page_id, tid, table_id, timestamp, buf, mode);
							
	if(deadlock_check(tid, page_id, table_id, mode))
	{
		abort_txn(ttxn);
		pthread_mutex_unlock(&locktable[table_id].ltmutex);
		return NULL;
	}	
	
	if(flags == 2)
	{
		if(!issleep(ttxn))
			gosleep(ttxn, tlock);
			
		pthread_mutex_unlock(&locktable[table_id].ltmutex);
		return tlock;		
	}
	else
	{	
		pthread_mutex_unlock(&locktable[table_id].ltmutex);
		return tlock;
	}
}

int issleep(txn_t * txn)
{
	pthread_mutex_lock(&txnlist.txn_list_mutex);
	if(txn->mode == WAITING)
		pthread_mutex_unlock(&txnlist.txn_list_mutex);
		return 1;

	pthread_mutex_unlock(&txnlist.txn_list_mutex);
	return 0;
}

int gosleep(txn_t * txn, lock_t * lock)
{
	pthread_mutex_lock(&txnlist.txn_list_mutex);
	txn->mode = WAITING;
	txn->wait_locks = lock;
	pthread_mutex_unlock(&txnlist.txn_list_mutex);

	return 1;
}

tHash * find_thash(int table_id, int page_id, int hash)
{
	tHash * thash = locktable[table_id].HashTable[hash];
	
	while(thash->next != NULL && thash->page_id != page_id)
		thash = thash->next;

	return thash;
}

DLNode * find_dlnode(tHash * thash, int temp)
{
	DLNode * dltemp = thash->head;
	
	while(dltemp->lock->txn->tid != temp && dltemp->next != NULL)
		dltemp = dltemp->next;		
	
	if(dltemp->lock->txn->tid == temp)
		return dltemp;
	else
		return NULL;
}

int deadlock_check(int tid, dbint page_id, int table_id, LMODE mode)
{
	int size = txnSize();
	int head = txnHead();
	int * visit = (int*)malloc(sizeof(int) * size);
	int temp = tid;
	int hash;
	txn_t * ttxn;
	lock_t * tlock;
	LNode * locknode;
	tHash * thash;
	DLNode * dltemp,* dltemp2;
	queue<int> q1;
	queue<DLNode*> q2, q3;
	
	memset(visit, 0, sizeof(int) * size);
	dltemp = thash->tail;	

	visit[temp] = 1;
	q1.push(temp);
	
	temp = q1.front();
	q1.pop();
	hash = page_id % HSIZE;
	thash = find_thash(table_id, hash);

	while(dltemp->prev != NULL)
	{
		dltemp = dltemp->prev;
		temp = dltemp->lock->txn->tid;
		if(visit[temp] != 1)
			q1.push(temp);
	}
	
	while(!q1.empty())
	{
		temp = q1.front();	
		q1.pop();
		ttxn = TXN_LFind(temp);

		locknode = ttxn->txn_locks;	

		while(locknode != NULL)
		{
			tlock = locknode->lock;
			hash = (tlock->page_id) % HSIZE;
			thash = find_thash(tlock->table_id, hash);
			dltemp = find_dlnode(thash, temp);
			if(dltemp->isvisit == 0)
			{
				dltemp->isvisit = 1;
				q2.push(dltemp);		
				q3.push(dltemp);
			}
			locknode = locknode->next;
		}
	}
	
	while(!q2.empty())
	{
		dltemp2 = q2.front();	
		q2.pop();
		dltemp2 = dltemp2->next;
		
		if(dltemp2->lock->page_id == page_id)
		{
			while(!q3.empty())
			{
				dltemp = q3.front();
				q3.pop();
				dltemp->isvisit = 0;
			}
			
			return DEADLOCK;			
		}

		while(dltemp2 != NULL)
		{
			ttxn = dltemp2->lock->txn;	
			locknode = ttxn->txn_locks;
			while(locknode != NULL)
			{
				tlock = locknode->lock;
				hash = (tlock->page_id) % HSIZE;
				thash = find_thash(tlock->table_id, hash);
				dltemp = find_dlnode(thash, temp);
				if(dltemp->isvisit = 0)	
				{
					dltemp->isvisit = 1;
					q2.push(dltemp);
					q3.push(dltemp);
				}
				locknode = locknode->next;
			}
			dltemp2 = dltemp2->next;
		}
	}
		
	while(!q3.empty())
	{
		dltemp = q3.front();
		q3.pop();
		dltemp->isvisit = 0;
	}
	return NODEADLOCK;
}

void putLog(txn_t * txn, int table_id ,dbint page_id, dbint record_id, dbint * olddata, dbint * newdata)
{
	pthread_mutex_lock(&txnlist.txn_list_mutex);
	txn->lognum++;
	if(txn->lognum == 1)
	{
		txn->logs = (Log*)malloc(sizeof(Log));
		txn->logs->table_id = table_id;
		txn->logs->page_id = page_id;
		txn->logs->record_id = record_id;
		txn->logs->olddata = olddata;
		txn->logs->newdata = newdata;
	}
	else
	{
		txn->logs = (Log*)realloc(txn->logs, sizeof(Log) * txn->lognum);
		txn->logs[txn->lognum - 1].table_id = table_id;
		txn->logs[txn->lognum - 1].page_id = page_id;
		txn->logs[txn->lognum - 1].record_id = record_id;
		txn->logs[txn->lognum - 1].olddata = olddata;
		txn->logs[txn->lognum - 1].newdata = newdata;
	}
	pthread_mutex_unlock(&txnlist.txn_list_mutex);
}

void delete_from_lock_table(txn_t * dtxn)
{
	LNode * temp = dtxn->txn_locks;
	lock_t * dlock;
	DLNode *dltemp, *dltemp2;
	tHash * thash;
	int table_id;

	while(temp != NULL)
	{
		table_id = temp->lock->table_id;	
		dlock = temp->lock;
		thash = find_thash(table_id, (dlock->page_id / HSIZE));
		dltemp = thash->head;	
		
		while(dltemp->next != NULL && dltemp->lock != dlock)
			dltemp = dltemp->next;	
		
		if(dltemp == thash->head)
		{
			thash->head = dltemp->next;	
			dltemp2 = dltemp->next;
			
			if(dltemp->lock->mode == EXCLUSIVE)
			{
				wake_up_txn(dltemp2);
				if(dltemp2->lock->mode == SHARED)
				{
					while(dltemp2->next != NULL && dltemp->lock->mode == SHARED)
					{
						dltemp2 = dltemp2->next;
						wake_up_txn(dltemp2);
					}
				}
			}
			else
			{
				if(dltemp2->lock->mode == EXCLUSIVE)
					wake_up_txn(dltemp2);
			}
			if(thash->head != NULL)
				thash->head->prev = NULL;
			
			dltemp->lock->buffer->is_pinned--;
			free(dltemp->lock);
			free(dltemp);
		}
		else if(dltemp == thash->tail)
		{
			thash->tail = dltemp->prev;
			thash->tail->next = NULL;	
	
			dltemp->lock->buffer->is_pinned--;
			free(dltemp->lock);
			free(dltemp);
		}	
		else
		{
			dltemp->prev->next = dltemp->next;
			dltemp->next->prev = dltemp->prev;

			dltemp->lock->buffer->is_pinned--;
			free(dltemp->lock);
			free(dltemp);
		}
	}
}

void recover_log(txn_t * txn)
{
	Page temp;	
	Log * logs = txn->logs;
	int num = txn->lognum;
	int i,j,k;

	for(i = 0 ; i < num ; i++)
	{
		buf_read_page(logs[i].page_id, &temp, logs[i].table_id);

		for(j = 0 ; j < temp.leafp.pheader.numOfKeys ;j++)
			if(temp.leafp.records[j].key == logs->record_id)
				break;

		for(k = 0 ; k < numOfCol[logs[i].table_id] - 1 ; k++)
			temp.leafp.records[j].values[k] = logs[i].newdata[k + 1];

		buf_write_page(logs[i].page_id, &temp, logs[i].table_id);
		free(logs[i].olddata);
		free(logs[i].newdata);
	}
	free(logs);
}

int abort_txn(txn_t * txn)
{
	recover_log(txn);	
	delete_from_lock_table(txn);	
	TXN_LDelete(txn->tid);
	free(txn);	

	return 1;
}

int force(txn_t * txn)
{
	LNode * locknode = txn->txn_locks, *temp;
	Page dest;
	int * isVisit;
	isVisit = (int*)malloc(sizeof(int) * (MAXTABLE + 1));	
	memset(isVisit, 0, sizeof(int) * (MAXTABLE + 1));
	
	while(locknode != NULL)
	{
		temp = locknode;
		buf_read_page(locknode->lock->page_id, &dest, locknode->lock->table_id);
		file_write_page(locknode->lock->page_id, &dest, locknode->lock->table_id);
		isVisit[locknode->lock->table_id] = 1;
		locknode = locknode->next;
		free(locknode->lock);
		free(locknode);
	}	
	
	for(int i = 0 ; i <= MAXTABLE ; i++)
		if(isVisit[i] == 1)
			fsync(default_fd[i]);

	free(isVisit);

	return 1;
}

int begin_tx()
{
	int newTxn = __sync_fetch_and_add(&txnNumber, 1);	
	txn_t * temp = makeTxn(newTxn);	
	TXN_LInsert(temp);

	return newTxn;
}

int end_tx(int tid)
{
	txn_t * dtxn = TXN_LDelete(tid);
	force(dtxn);
	delete_from_lock_table(dtxn);	
	TXN_LDelete(tid);

	free(dtxn->logs);
	free(dtxn);	

	return 1;
}
