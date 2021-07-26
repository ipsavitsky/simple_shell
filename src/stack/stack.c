#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SAFE(call)             \
    do {                           \
        if ((call) == NULL) {      \
            stack_finalize(stack); \
            return E_MEM_ALLOC;    \
        }                          \
    } while (0)

int stack_init(Stack *stack, size_t data_size) {
    MEM_SAFE(stack->data = malloc(data_size));
    stack->stack_top = stack->data;
    stack->stack_size = data_size;
    stack->cur_size = 0;
    return 0;
}

void stack_finalize(Stack *stack) { free(stack->data); }

int stack_pop(Stack *stack, void *resp, size_t size_res) {
    void *cur = stack->stack_top;
    // get size of an element
    cur = (char *)cur - sizeof(Size_elem);
    if (cur < stack->data) return E_UNDERFLOW;
    size_t size = *((char *)cur);
    if (size > size_res) return E_OVERFLOW;
    // get the element itself
    cur = (char *)cur - size;
    if (cur < stack->data) return E_UNDERFLOW;
    memcpy(resp, cur, size);
    stack->cur_size -= size;
    stack->stack_top = cur;
    return 0;
}

int stack_push(Stack *stack, const void *resp, size_t size_res) {
    // printf("PUSHING %ld\n", size_res);
    if (stack->cur_size + size_res + sizeof(Size_elem) > stack->stack_size) return E_OVERFLOW;
    stack->cur_size += size_res + sizeof(Size_elem);

    memcpy((char *)stack->stack_top, resp, size_res);
    memcpy((char *)stack->stack_top + size_res, &size_res, sizeof(Size_elem));

    stack->stack_top = &(((char *)stack->stack_top)[size_res + sizeof(Size_elem)]);
    return 0;
}
