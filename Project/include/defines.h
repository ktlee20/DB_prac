#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <stdint.h>

#define PAGESIZE 4096
#define LBRFACTOR 32
#define IBRFACTOR 249
#define VALUESIZE 120
#define MAXTABLE 10
#define RESERVEDSIZE 104
#define MAXVALNUM 15
#define INITTREE 20
#define MAXJOINNUM 10

#define FAIL 0xFFFFFFFFFFFFFFFF

#define FALSE 0
#define TRUE 1

#define CUT(x) ((x) / 2)
#define _ROOTP header->pheader.rootp_offset

typedef int64_t dbint;
typedef dbint pagenum_t;

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
	union _pages_ frame;
	int table_id;
	dbint page_num;
	int is_dirty;
	int is_pinned;
	struct _buf_struct * next;
	struct _buf_struct * prev;
} Bufstrt;

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

typedef struct queue
{
	int numOfData;
	QNode * front;
	QNode * rear;
} Queue;

typedef struct jtqsort
{
	dbint ** jt;
	int col;
	int length;
} T0;

#endif
