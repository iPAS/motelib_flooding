#include "linklist.h"

#include <stdio.h>

void ll_init(linklist_data_st *hdl)
{
    hdl->first = NULL;
    hdl->last = NULL;
    hdl->len = 0;
}
