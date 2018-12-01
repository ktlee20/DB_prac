#ifndef __LIB_BPT_H_
#define __LIB_BPT_H_

//defines
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>

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

typedef struct queue
{
	int numOfData;
	QNode * front;
	QNode * rear;
} Queue;

typedef struct _t_qsort
{
	dbint ** jt;
	int col;
	int length;
} T0;


//globals
extern FILE ** default_file;
extern int * default_fd;
extern Page ** header;
extern Page ** rootpage;
extern pagenum_t * totalp;
extern pagenum_t * currentp;

//BufferManager.c
extern Bufstrt * buffer[MAXTABLE+1];
extern Bufstrt * bufferlast[MAXTABLE+1];
extern int bufsize;
extern int curPos[MAXTABLE+1];
extern int numOfCol[MAXTABLE+1];

//Optimizer.c
extern OptInfo * oinfo[MAXTABLE+1];
extern JTable * memory_key[MAXTABLE+1];

//join.c
extern int printJTable;
extern pthread_t thread_t[MAXJOINNUM + 1][MAXVALNUM + 1];


//join
PNode * parse(char * input);
void * tt_JTqsort(void * data);
void* t_JTqsort(void * data,int table_id);
void JTqsort(dbint** jt, int col,int length);
void cusFree(JTable * jt);
JTable * joining(PNode * left, PNode * right, JTable * jtleft, JTable * jtright);
JTable * join_table(PNode * tree);
dbint join(char const* str);


//dbinit
void globalInit(int);
int empty();
int clearele(int);
int headerInit(int,int);
void dbend(int);

//dbbpt
typedef Page node;
node * dbinsert(node * root, dbint key, dbint* value,int p);
Records * dbfind(node * root, dbint key,pagenum_t* poffset,int p);
node * find_leaf(node * root, dbint key,pagenum_t* poffset,int p);
void Insert_value(dbint * value, int tnum, Records * record);
Records * make_records(dbint* value, int tnum);
node * start_new_tree(dbint key, Records* pointer,int i);
node * make_leaf(pagenum_t parent_off, pagenum_t otherp_off);
node * make_node(pagenum_t parent_off, pagenum_t otherp_off);
node * insert_into_leaf(node * leaf, dbint key, Records* pointer,pagenum_t offset,int j);
node * insert_into_leaf_after_splitting(node * leaf, dbint key, Records * pointer, pagenum_t old_offset,int p);
int get_left_index(node * parent, pagenum_t loffset);
node * insert_into_new_root(node * left, pagenum_t loffset, dbint key, node * right, pagenum_t roffset,int p);
node * insert_into_node(node *n, int left_index, dbint key, node * right, pagenum_t roffset,int j);
node * insert_into_parent(node * left, pagenum_t loffset, dbint key, node * right, pagenum_t roffset,int i);
node * insert_into_node_after_splitting(node * old_node, pagenum_t old_offset, int left_index, dbint key, node * right, pagenum_t roffset, int p);
node * dbdelete(node * root, dbint key,int p);
node * delete_entry(node * n, dbint key, void * pointer, pagenum_t offset,int p);
int get_neighbor_index(node * n, pagenum_t offset, pagenum_t * neighbor_offset,int p);
node * adjust_root(node * root,int p);
node * remove_entry_from_node(node * n, dbint key, void * pointer, pagenum_t offset,int p);
node * coalesce_nodes(node * n, pagenum_t curoffset, pagenum_t noffset, int n_index,int p);

//Optimizer
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


//FileIndexManager
int init_db(int);
int open_table(char * pathname,int num_column);
int close_table(int);
int shutdown_db();
int insert(int table_id, dbint key, dbint* values);
dbint* find(int table_id, dbint key);
int erase(int table_id, dbint key);
pagenum_t getPageId(int table_id, dbint key);


//DiskManager
int make_new_header(int,int);
int chgheadroff(pagenum_t rootpnum,int i);
pagenum_t file_alloc_page(int);
void file_free_page(pagenum_t pagenum, int i);
void file_read_page(pagenum_t pagenum, Page * dest,int i);
void file_write_page(pagenum_t pagenum, const Page * src,int i);

//BufferManager
Bufstrt * makeBufNode();
void bufferAttr(Bufstrt* node, dbint offset, int table_id);
void newBuf(int new_buf, int table_id);
void closeBuf(int table_id);
pagenum_t get_page_offset(Bufstrt * node);
Bufstrt * find_leaf_buffer(pagenum_t offset, int table_id);
Page * set_leaf_page(Page * dest, Page * src);
Page * delete_leaf_buffer(Page * dest, dbint key);
Page * insert_into_buffer(Page * dest, dbint key, char * pointer);
void buf_flush_page(pagenum_t offset, int table_id);
Page * buf_read_page(pagenum_t offset, Page * dest ,int table_id);
void buf_write_page(pagenum_t offset, Page * src, int table_id);
void buf_lru_set(Bufstrt * node,int table_id);


#endif
