#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <stdint.h>

#define PAGESIZE 4096
#define LBRFACTOR 32
#define IBRFACTOR 249
#define VALUESIZE 120
#define MAXTABLE 10
#define RESERVEDSIZE 104

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
	char value[VALUESIZE];
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
	char reserved[PAGESIZE - (sizeof(dbint)*3)];
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

#endif
