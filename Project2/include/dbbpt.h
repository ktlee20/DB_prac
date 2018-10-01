#ifndef __DISK_BASE_BPT_H_
#define __DISK_BASE_BPT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "defines.h"

typedef uint64_t dbint;
typedef dbint pagenum_t;
typedef Page node;

void findleaf(node * leaf, int key);
char* find(node * leaf, int key);
int findkey(node* leaf, int key);
Records* make_record(char * value);
int insert(int key, char * value);

#endif
