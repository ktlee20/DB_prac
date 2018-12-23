#ifndef __LOCK_MGR_H__
#define __LOCK_MGR_H__

#include "defines.h"

txn_t * makeTxn(int tid);
lock_t * makeLock(int table_id, txn_t * txn, dbint page_id, uint64_t timestamp, LMODE mode, Bufstrt * buf);
DLNode * makeDLNode(lock_t * lock);
void TXN_LInsert(txn_t * txn);
txn_t * TXN_LFind(int tid);
int txnSize();
int txnHead();
int txnTail();
txn_t * TXN_LDelete(int tid);
DLNode * dl_find_lock_table(dbint page_id, int tid, int table_id, LMODE mode);
lock_t * find_lock_table(dbint page_id, int tid, int table_id, LMODE mode);
int begin_tx();
int end_tx(int tid);
void TXN_LInsert(txn_t * txn);
txn_t * TXN_LDelete(int tid);
int set_lock(txn_t * txn, dbint page_id, int tid, int table_id, uint64_t timestamp, Bufstrt * buf, LMODE mode);
lock_t * insert_lock_table(dbint page_id, int tid, int table_id, uint64_t timestamp, LMODE mode);
tHash * find_thash(int table_id, int hash);
DLNode * find_dlnode(tHash * thash, int temp);
int deadlock_check(int tid, dbint page_id, int table_id, LMODE mode);
int issleep(txn_t * txn);
int gosleep(txn_t * txn, lock_t * lock);
tHash * find_thash(int table_id, int hash);
void delete_from_lock_table(txn_t * dtxn);
void recover_log(txn_t * txn);
int wake_up_txn(DLNode * dltemp);
int abort_txn(txn_t * txn);
int force(txn_t * txn);
void putLog(txn_t * txn, int table_id, dbint page_id, dbint record_id, dbint * olddata, dbint * newdata);

#endif
