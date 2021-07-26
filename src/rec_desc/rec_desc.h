#include "RPN.h"
/**
 * structure of an infex expression
 */
typedef struct {
    unsigned char *string_form;  ///< string with the expression
    unsigned char *curpointer;   ///< pointer to the current symbol(used internally)
} Expression;

int compute_expression(Expression *expr, int *res);
int init_expression(Expression *expr, char *input);
void finalize_expression(Expression *expr);
int add_variable_to_table(Expression *expr, const char *name, double num);
