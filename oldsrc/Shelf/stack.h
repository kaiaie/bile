#ifndef _STACK_H
#define _STACK_H
#include <stddef.h>
#include "bool.h"
#include "list.h"

#define INVALID_STACK INVALID_LIST
typedef List Stack;

List new_Stack();
void delete_Stack(Stack s);

bool Stack_dup(Stack s);
bool Stack_pop(Stack s, int *v);
bool Stack_push(Stack s, int v);

bool Stack_getLength(Stack s, size_t *lgt);

#endif
