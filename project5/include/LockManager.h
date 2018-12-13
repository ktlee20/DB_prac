#ifndef __LOCK_MGR_H__
#define __LOCK_MGR_H__

#include "defines.h"

txn_t * makeTXN(int tid);
lock_t * makeLOCK(int table_id, dbint key, txn_t * txn, dbint page_id, uint64_t timestamp, LMODE mode);
DLNode * makeDLNode(lock_t * locks);
int begin_tx();
int end_tx(int tid);
void TXN_LInsert(txn_t * txn);
txn_t * TXN_LDelete(int tid);
txn_t * TXN_LFind(int tid);
txn_t * TXN_LFind_LockInsert(int tid, lock_t * lock);
LMODE Insert_Lock_table(dbint page_id, int tid, dbint key, int table_id, uint64_t timestamp, LMODE mode);
LMODE get_lock_mode_in_lock(int hash, dbint key, int table_id);

#endif
