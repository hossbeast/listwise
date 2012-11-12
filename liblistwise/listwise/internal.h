#ifndef _LISTWISE_INTERNAL_H
#define _LISTWISE_INTERNAL_H

#include "listwise.h"
#include "listwise/operator.h"
#include "listwise/generator.h"
#include "listwise/lstack.h"
#include "listwise/ops.h"
#include "listwise/object.h"

#define API __attribute__((visibility("protected")))
#define APIDATA

int op_load(char* path);
void op_sort();
void op_teardown();

#endif
