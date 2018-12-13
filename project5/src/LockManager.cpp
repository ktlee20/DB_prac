#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "defines.h"
#include "globals.h"
#include "LockManager.h"

int txnNumber;
LockTable locktable[MAXTABLE + 1];
TXNList txnlist;
uint64_t timer[MAXTABLE + 1];

txn_t * makeTXN(int tid)
{
	txn_t * retTxn = (txn_t*)malloc(sizeof(txn_t));
	retTxn->tid = tid;
	retTxn->state = EMBRYO;
	retTxn->txn_locks = NULL;
	retTxn->wait_locks = NULL;

	return retTxn;
}

lock_t * makeLOCK(int table_id, dbint key, txn_t * txn, dbint page_id, uint64_t timestamp, LMODE mode)
{
	lock_t * retLock = (lock_t*)malloc(sizeof(lock_t));
	retLock->table_id = table_id;
	retLock->key = key;
	retLock->txn = txn;
	retLock->page_id = page_id;
	retLock->timestamp = timestamp;
	retLock->mode = mode;
	retLock->cond = PTHREAD_COND_INITIALIZER;

	return retLock;
}

DLNode * makeDLNode(lock_t * lockt)
{
	DLNode * retDLNode = (DLNode*)malloc(sizeof(DLNode));
	retDLNode->lock = lockt;
	retDLNode->next = NULL;
	retDLNode->prev = NULL;

	return retDLNode;
}

void TXN_LInsert(txn_t * txn)
{
	TXNLNode *temp, *newNode;

	pthread_mutex_lock(&txnlist.txn_list_mutex);			
	if(txnlist.head == NULL)
	{
		txnlist.head = txnlist.tail = (TXNLNode*)malloc(sizeof(TXNList));
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
			while(temp->prev != NULL && temp->prev->txn->tid > txn->tid)
				temp = temp->prev;

			if(temp->prev == NULL)
			{
				newNode->next = temp;	
				temp->prev = newNode;
				newNode->prev = NULL;
				txnlist.head = newNode;
			}
			else
			{
				newNode->next = temp;
				newNode->prev = temp->prev;
				temp->prev->next = newNode;
				temp->prev = newNode;
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
			
	pthred_mutex_unlock(&txnlist.txn_list_mutex);
	return temp;
}

txn_t * TNX_LFind_LockInsert(int tid, lock_t * lock)
{
	TXLNode * temp;
	LNode * temp_locks;

	pthread_mutex_lock(&txnlist.txn_list_mutex);
	temp = txnlist.head;
	
	if(temp == NULL)
	{
		pthread_mutex_unlock(&txnlist.txn_list_mutex);
		return NULL;
	}

	while(temp->txn->tid != tid && temp->next != NULL)	
		temp = temp->next;
	
	if(temp->txn->tid == tid)
	{
		if(temp->txn->txnlocks == NULL)
		{
			temp->txn->txnlocks = (LNode*)malloc(sizeof(LNode));
			temp->txn->txnlocks->lock = lock;
			temp->txn->txnlocks->next = NULL;
		}
		else
		{
			temp_locks = temp->txn->txnlocks;
			while(temp_locks->next != NULL)
				temp_locks = temp_locks->next;	
			temp_locks->next = (LNode*malloc(sizeof(LNode));
			temp_locks->next->next = NULL;
			temp_locks->next->lock = lock;
		}
		pthread_mutex_unlock(&txnlist.txn_list_mutex);
		return temp->txn;
	}	
	else
	{
		pthread_mutex_unlock(&txnlist.txn_list_mutex);
		return NULL;
	}
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
			while(temp->next != NULL && temp->next->txn->tid != tid)
				temp = temp->next;

			if(temp->next == NULL)
				retTxn = NULL;
			else
			{
				temp2 = temp->next;	
				temp->next = temp2->next;
				if(temp->next != NULL)
					temp2->next->prev = temp;
				else
					txnlist.tail = temp;
				retTxn = temp->next->txn;
				free(temp);
			}
		}
	}
	pthread_mutex_unlock(&txnlist.txn_list_mutex);
	
	return retTxn;
}

int begin_tx()
{
	int adder = 1;
	int newTxn = __sync_fetch_and_add(&txnNumber, adder);
	txn_t * temp = makeTXN(newTxn);
	TXN_LInsert(temp);	
	
	return newTxn;
}

int end_tx(int tid)
{
	
}

LMODE get_lock_mode_in_lock(int hash, dbint key,int table_id)
{
	Hash * thash = locktable[table_id].HashTable[hash];
	DLNode * iter;	

	while(thash->key != key && thash->next != NULL)
		thash = thash->next; 

	iter = thash->tail;

	while(iter->lock->mode != EXCLUSIVE && iter->prev != NULL)
		iter = iter->prev;
	
	if(iter->lock->mode == EXCLUSIVE)
		return EXCLUSIVE;
	else
		return SHARED;
}


lock_t * Insert_Lock_Table(dbint page_id, int tid, dbint key, int table_id, uint64_t timestamp, LMODE mode, LMODE * pMode)
{
	int hash;
	Hash * thash;
	txn_t * insertTxn = TXN_LFind(tid);
	lock_t * newLock = makeLOCK(table_id, key, insertTxn, page_id, timestemp, mode);
	hash = (key + page_id) / (HSIZE + 1);

	pthread_mutex_lock(&locktable[table_id].ltmutex);

	if(locktable[table_id].HashTable[hash] == NULL)
	{
		locktable[table_id].HashTable[hash] = (Hash*)malloc(sizeof(Hash));
		locktable[table_id].HashTable[hash]->key = key;
		locktable[table_id].HashTable[hash]->head = locktable[table_id].HashTable[hash]->tail = makeDLNode(newLock);
		locktable[table_id].HashTable[hash]->next = NULL;
	}
	else
	{
		thash = locktable[table_id].HashTable[hash];	
		while(thash->key != key && thash->next != NULL)
			thash = thash->next;	
	
		if(thash->key == key)
		{
			locktable[table_id].HashTable[hash].tail->next = makeDLNode(newLock);		
			locktable[table_id].HashTable[hash].tail->next->prev = locktable[table_id].HashTable[hash].tail;
			locktable[table_id].HashTable[hash].tail = locktable[table_id].HashTable[hash].tail->next;
		}
		else
		{
			thash->next = (Hash*)malloc(sizeof(Hash));
			thash->next->key = key;
			thash->next->head = thash->next->tail = makeDLNode(newLock);
			thash->next->next = NULL;
		}
	}
	*pMode = get_lock_mode_in_lock(hash, key, table_id);	
	
	pthread_mutex_unlock(&locktable[table_id].ltmutex);
	
	return newLock;
}
