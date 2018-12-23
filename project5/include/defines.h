#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <stdint.h>
#include <pthread.h>

#define PAGESIZE 4096
#define LBRFACTOR 32
#define IBRFACTOR 249
#define VALUESIZE 120
#define MAXTABLE 10
#define RESERVEDSIZE 104
#define MAXVALNUM 15
#define INITTREE 20
#define MAXJOINNUM 10
#define QSTHREAD 8
#define HSIZE 9997

#define FAIL 0xFFFFFFFFFFFFFFFF

#define FALSE 0
#define TRUE 1
#define DEADLOCK 1
#define NODEADLOCK 0
#define SUCCESS 1
#define FAILED 0

#define CUT(x) ((x) / 2)
#define _ROOTP header->pheader.rootp_offset

typedef int64_t dbint;
typedef dbint pagenum_t;

typedef enum lock_mode { SHARED, EXCLUSIVE } LMODE;
typedef enum txn_state { EMBRYO, IDLE, RUNNING, WAITING , DEAD} TMODE;

typedef struct page_header
{
	dbint parent_page;
	int is_leaf;
	int numOfKeys;
	char reserved[RESERVEDSIZE];
	dbint other_page_offset;
} Page_Header;

typedef struct _records
{
	dbint key;
	dbint values[MAXVALNUM];
} Records;

typedef struct ientity
{
	dbint key;
	dbint offset;
} Entity;

typedef struct header_page
{
	dbint freep_offset;
	dbint rootp_offset;
	dbint numOfPage;
	dbint numOfCol;
	char reserved[PAGESIZE - (sizeof(dbint)*4)];
} HeaderPage;

typedef struct free_page
{
	dbint next_freep;
	char reserved[PAGESIZE - sizeof(dbint)];
} FreePage;

typedef struct leaf_page
{
	struct page_header pheader;
	struct _records records[LBRFACTOR - 1];
} LeafPage;

typedef struct internal_page
{
	struct page_header pheader;
	struct ientity entities[IBRFACTOR-1];	
} InternalPage;

typedef union _pages_
{
	struct header_page headerp;
	struct free_page freep;
	struct leaf_page leafp;
	struct internal_page internalp;
} Page;


//Buffer
typedef struct _buf_struct
{
	int table_id;
	int is_dirty;
	int is_pinned;
	struct _buf_struct * next;
	struct _buf_struct * prev;
	dbint page_num;
	union _pages_ frame;
} Bufstrt;

typedef struct _buf_strt_cont
{
	pthread_mutex_t buf_ctrl_mutex;	
	Bufstrt * buffer;
	Bufstrt * bufferlast;
	int bufsize;
	int curPos;
} Buffer_Control_Block;

//Optimizer
typedef struct _opt_min_max_
{
	dbint min;
	dbint max;
} MinMax;

typedef struct _optimal_info_
{
	dbint num;
	MinMax mm[MAXVALNUM + 1];	
} OptInfo;

//In-memory struct
typedef struct _join_table__
{
	dbint ** iArr;
	dbint numOfData;
	dbint arrSize;
	int joinedNum; 
	int colSize[MAXTABLE];
	int tid[MAXTABLE];
} JTable;

typedef struct _ptnode_
{
	int table_id;
	int col;
	int sorted;
	int qsnum;
	int tt[QSTHREAD];
	int cc[QSTHREAD];
	struct _ptnode_ *left;
	struct _ptnode_ *right;
	struct _ptnode_ *parent;
} PNode;

typedef int QData;

typedef struct _qnode
{
	QData data;
	struct _qnode * next;
	struct _qnode * prev;
}	QNode;

typedef struct __queue_
{
	int numOfData;
	QNode * front;
	QNode * rear;
} tQueue;

typedef struct _t_qsort
{
	dbint ** jt;
	int col;
	int length;
} T0;

typedef struct _lnode_
{
	struct _lnode_ * next;	
	struct __lock_ * lock;
} LNode;

typedef struct _dlnode_
{
	int isvisit;
	struct __lock_ *lock;	
	struct _dlnode_ *next;
	struct _dlnode_ *prev;
} DLNode;

typedef struct _hash_slot_
{
	int page_id;	
	struct _dlnode_ *head;
	struct _dlnode_ *tail;
	struct _hash_slot_ *next;
} tHash;

typedef struct _txn_link_
{
	struct _txn_link_ *next;  
	struct _txn_link_ *prev;
	struct __txn_ * txn;
} TXNLNode;

typedef struct _txn_linked_list
{
	pthread_mutex_t txn_list_mutex;
	TXNLNode * head;
	TXNLNode * tail;
} TXNList;

typedef struct _lock_table_
{
	pthread_mutex_t ltmutex;	
	tHash * HashTable[HSIZE];
} LockTable;

typedef struct _log_
{
	dbint table_id;
	dbint page_id;
	dbint record_id;
	dbint * olddata;
	dbint * newdata;
} Log;

typedef struct __txn_
{
	int tid;
	TMODE mode;
	LNode * txn_locks;
	struct __lock_* wait_locks;
	int lognum;
	Log * logs;
} txn_t;

typedef struct __lock_ 
{
	int table_id;
	LMODE mode;	
	struct __txn_* txn;
	pthread_cond_t cond;
	dbint page_id;
	uint64_t timestamp;
	Bufstrt * buffer;
	pthread_mutex_t lock_mutex;
} lock_t;

#endif
