#include <stddef.h>

#include "errors.h"

/**
 * structure of a stack
 */
typedef struct {
    void *data;         ///< data of a stack
    void *stack_top;    ///< pointer to the end of a valid stack
    size_t stack_size;  ///< size of a stack(not always to the last element)
    size_t cur_size;  ///< control of the current sie of stack(used internally)
} Stack;

typedef unsigned short Size_elem;

/**
 * initialize a stack
 * \param stack stack to initialize
 * \param data_size maximum size of a stack
 * \exception E_MEM_ALLOC Thrown in case of memory allocation error
 * \return error code
 */
int stack_init(Stack *stack, size_t data_size);

/**
 * finalize a stack
 * \param stack stack to finalize
 */
void stack_finalize(Stack *stack);

/**
 * pop an element to the stack
 * \param stack stack from which to pop
 * \param resp array to which write the result
 * \param size_res size of resp. In case of overflow returns an error
 * \exception E_UNDERFLOW Thrown in case of popping empty or corrupted stack
 * \exception E_OVERFLOW Thrown if the popped memory overflows resp
 * \warning it is up to the caller to allocate memory for `resp`
 * \return error code
 */
int stack_pop(Stack *stack, void *resp, size_t size_res);

/**
 * push an element to the stack
 * \param stack stack to which to push
 * \param resp array to which write the result
 * \param size_res size of resp. In case of overflow returns an error
 * \exception E_OVERFLOW Thrown in case ov overflow of allocated memory
 */
int stack_push(Stack *stack, const void *resp, size_t size_res);
