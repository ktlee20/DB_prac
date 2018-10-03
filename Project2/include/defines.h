#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <stdint.h>

#define PAGESIZE 4096
#define LBRFACTOR 32
#define IBRFACTOR 249
#define VALUESIZE 120

#define FALSE 0
#define TRUE 1

#define CUT(x) ((x) / 2)

typedef int64_t dbint;
typedef dbint pagenum_t;

typedef struct page_header
{
	dbint parent_page;
	int is_leaf;
	int numOfKeys;
	char reserved[104];
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
	struct _records records[LBRFACTOR];
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
#endif
