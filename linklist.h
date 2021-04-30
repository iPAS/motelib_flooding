#ifndef __LINKLIST_H__
#define __LINKLIST_H__


#include <stdint.h>
#include <string.h>

typedef struct linklist_st
{
    void *data;
    struct linklist_st *next;
} linklist_st;

typedef struct
{
    linklist_st *first;
    linklist_st *last;
    uint8_t len;
} linklist_data_st;

extern void ll_init(linklist_data_st *hdl);


#endif  // __LINKLIST_H__
