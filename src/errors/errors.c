#include "errors.h"
#include <stdio.h>

void err_print(int err){
    fprintf(stderr, "\033[31;1;4mError:\033[0m");
    switch (err){
        case E_MEM_ALLOC:
            fprintf(stderr, " memory allocation\n");
            break;
        case E_OVERFLOW:
            fprintf(stderr, " dynamic structure overflow\n");
            break;
        case E_UNDERFLOW:
            fprintf(stderr, " dynamic structure underflow\n");
            break;
        case E_UNEXPECTED_SYMBOL:
            fprintf(stderr, " unexpected symbol\n");
            break;
        case E_ZERO_DIVISION:
            fprintf(stderr, " zero division prohibited\n");
            break;
        case E_UNKNOWN_VAR:
            fprintf(stderr, " unknown variable\n");
            break;
        case E_UNBALANCED_LB:
            fprintf(stderr, " unbalanced \'(\'\n");
            break;
        case E_UNBALANCED_RB:
            fprintf(stderr, " unbalanced \')\'\n");
            break;
        case E_MISSED_OPERATOR:
            fprintf(stderr, " missed operator\n");
            break;
        default:
            fprintf(stderr, " unknown(%d)\n", err);
            break;
    }
}