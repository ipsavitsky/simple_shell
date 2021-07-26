#include "funcs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define SAFE(call)                           \
    do {                                     \
        if ((flag = call) != 0) return flag; \
    } while (0)


int put_elem_in_RPN(RPN *expression, Size_elem size, void *data, Calculate_elem func);

int parse_logic(Expression *expr, RPN *stack_mach, size_t *br_inj);

int parse_redir(Expression *expr, RPN *stack_mach, long *infd, long *outfd, size_t *br_injection);

int parse_streamable_prog(Expression *expr, RPN *stack_mach, size_t *br_injection);

int parse_prog_call(Expression *expr, RPN *stack_mach);

int parse_literal(Expression *expr, RPN *stack_mach);

int parse_output_filename_rw(Expression *expr, long *fd);

int parse_output_filename_appnd(Expression *expr, long *fd);

int parse_input_filename(Expression *expr, long *fd);

int parse_pipes(Expression *expr, RPN *stack_mach, size_t *br_injection);

int put_elem_in_RPN(RPN *expression, Size_elem size, void *data, Calculate_elem func) {
    if (expression->occupied + sizeof(Size_elem) + size >= expression->data_size) {
        return E_OVERFLOW;
    }
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    } dat;
    dat.size = size;
    if (dat.size % 8 != 0) dat.size += (8 - size % 8);
    dat.f = func;
    // printf("{%d; %p}\n", dat.size, dat.f);
    memcpy((struct input_data *) &(((char *) expression->data)[expression->occupied]), &dat,
           sizeof(struct input_data));
    expression->occupied += sizeof(struct input_data);
    if (data != NULL) {
        memcpy((char *) expression->data + expression->occupied, data, size);
        expression->occupied += dat.size;
    }
    return 0;
}

int inject_value(RPN *expression, Size_elem size, void *data, size_t position) {
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    } *dat;
    dat = (struct input_data *) &((char *) expression->data)[position];
    dat->size = size;
    position += sizeof(struct input_data);
    memcpy((char *) expression->data + position, data, size);
    return 0;
}

int inject_size(RPN *expression, Size_elem size, size_t position) {
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    } *dat;
    dat = (struct input_data *) &((char *) expression->data)[position];
    dat->size = size;
    return 0;
}

int parse_logic(Expression *expr, RPN *stack_mach, size_t *br_inj) {
    int flag;
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    };
    size_t br_injection = -1;
    SAFE(parse_pipes(expr, stack_mach, &br_injection));
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    while ((strncmp(expr->curpointer, "&&", 2) == 0) || (strncmp(expr->curpointer, "||", 2) == 0) ||
           *(expr->curpointer) == ';') {
        size_t injection_point = stack_mach->occupied;
        size_t injection_value = 0;
        size_t dummy = 0;
        switch (*(expr->curpointer++)) {
            case '&':
                expr->curpointer++;
                if (br_injection == -1) {
                    SAFE(put_elem_in_RPN(stack_mach, 0, NULL, execute));
                } else {
                    SAFE(put_elem_in_RPN(stack_mach, sizeof(br_injection), &br_injection, execute_brackets));
                    injection_value += sizeof(br_injection);
                    br_injection = -1;
                }
                injection_point += sizeof(struct input_data);
                SAFE(put_elem_in_RPN(stack_mach, sizeof(dummy), &dummy, logic_and));
                dummy = 1;
                break;
            case '|':
                expr->curpointer++;
                if (br_injection == -1) {
                    SAFE(put_elem_in_RPN(stack_mach, 0, NULL, execute));
                } else {
                    SAFE(put_elem_in_RPN(stack_mach, sizeof(br_injection), &br_injection, execute_brackets));
                    br_injection = -1;
                }
                injection_point += sizeof(struct input_data);
                SAFE(put_elem_in_RPN(stack_mach, sizeof(dummy), &dummy, logic_or));
                dummy = 1;
                break;
            case ';':
                if (br_injection == -1) {
                    SAFE(put_elem_in_RPN(stack_mach, 0, NULL, execute));
                } else {
                    SAFE(put_elem_in_RPN(stack_mach, sizeof(br_injection), &br_injection, execute_brackets));
                    br_injection = -1;
                }
                break;
            default:
                break;
        }
        SAFE(parse_pipes(expr, stack_mach, &br_injection));
        if (br_injection != -1) {
            injection_value += sizeof(br_injection);
        }
        if (dummy == 1) {
            injection_value += stack_mach->occupied + sizeof(struct input_data);
            inject_value(stack_mach, sizeof(injection_value), &injection_value, injection_point);
        }
    }
    *br_inj = br_injection;
    return 0;
}


int parse_pipes(Expression *expr, RPN *stack_mach, size_t *br_injection) {
    int flag;
    long infd = 0, outfd = 1, safe = 1;
    // size_t injection_val = -1;
    SAFE(parse_redir(expr, stack_mach, &infd, &outfd, br_injection));
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    while ((*(expr->curpointer) == '|') && (*(expr->curpointer + 1) != '|')) {
        expr->curpointer++;
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            return E_PIPE;
        }
        safe = outfd = pipe_fd[1];
        SAFE(put_elem_in_RPN(stack_mach, sizeof(outfd), &outfd, chg_out_stream));
        SAFE(put_elem_in_RPN(stack_mach, sizeof(infd), &infd, chg_in_stream));
        if (*br_injection == -1) {
            SAFE(put_elem_in_RPN(stack_mach, 0, NULL, execute_nonblock));
        } else {
            SAFE(put_elem_in_RPN(stack_mach, sizeof(*br_injection), br_injection, execute_brackets_nonblock));
            *br_injection = -1;
        }
        infd = pipe_fd[0];
        SAFE(parse_redir(expr, stack_mach, &infd, &outfd, br_injection));
    }
    if (safe == outfd) {
        outfd = 1;
    }
    SAFE(put_elem_in_RPN(stack_mach, sizeof(outfd), &outfd, chg_out_stream));
    SAFE(put_elem_in_RPN(stack_mach, sizeof(infd), &infd, chg_in_stream));
    return 0;
}


int parse_redir(Expression *expr, RPN *stack_mach, long *infd, long *outfd, size_t *br_injection) {
    int flag;
    SAFE(parse_streamable_prog(expr, stack_mach, br_injection));
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    while ((strncmp(expr->curpointer, ">", 1) == 0) ||
           (strncmp(expr->curpointer, "<", 1) == 0) || (strncmp(expr->curpointer, ">>", 2) == 0)) {
        if (*(expr->curpointer) == '>') {
            if (*(expr->curpointer + 1) == '>') {
                expr->curpointer++;
                SAFE(parse_output_filename_appnd(expr, outfd));
                // outflag = 1;
            } else {
                SAFE(parse_output_filename_rw(expr, outfd));
                // outflag = 2;
            }
        } else if (*(expr->curpointer) == '<') {
            SAFE(parse_input_filename(expr, infd));
            // inflag = 3;
        }
    }
    return 0;
}

int parse_input_filename(Expression *expr, long *fd) {
    int size = 0;
    unsigned char var[129];
    expr->curpointer++;
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    while (((*(expr->curpointer) != '|') && (*(expr->curpointer) != ';') && (*(expr->curpointer) != '>') &&
            (*(expr->curpointer) != '<') &&
            (*(expr->curpointer) != '\0')) &&
           (size < 128))
        var[size++] = *(expr->curpointer++);
    var[size - 1] = '\0';
    long loc_fd;
    if ((loc_fd = open(var, O_RDONLY)) == -1) {
        return E_FILE_OP;
    }
    *fd = loc_fd;
    return 0;
}

int parse_output_filename_rw(Expression *expr, long *fd) {
    int size = 0;
    unsigned char var[129];
    expr->curpointer++;
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    while (((*(expr->curpointer) != '|') && (*(expr->curpointer) != ';') && (*(expr->curpointer) != '>') &&
            (*(expr->curpointer) != '<') &&
            (*(expr->curpointer) != '\0')) &&
           (size < 128))
        var[size++] = *(expr->curpointer++);
    var[size - 1] = '\0';
    long loc_fd;
    if ((loc_fd = creat(var, 0644)) == -1) {
        return E_FILE_OP;
    }
    *fd = loc_fd;
    return 0;
}

int parse_output_filename_appnd(Expression *expr, long *fd) {
    int size = 0;
    unsigned char var[129];
    expr->curpointer++;
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    while (((*(expr->curpointer) != '|') && (*(expr->curpointer) != ';') && (*(expr->curpointer) != '>') &&
            (*(expr->curpointer) != '<') &&
            (*(expr->curpointer) != '\0')) &&
           (size < 128))
        var[size++] = *(expr->curpointer++);
    var[size - 1] = '\0';
    long loc_fd;
    if ((loc_fd = open(var, O_CREAT | O_APPEND | O_WRONLY, 0644)) == -1) {
        return E_FILE_OP;
    }
    *fd = loc_fd;
    return 0;
}

int parse_streamable_prog(Expression *expr, RPN *stack_mach, size_t *br_injection) {
    int flag;
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    };
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    if (*(expr->curpointer) == '(') {
        ++(expr->curpointer);
        size_t injection_point = stack_mach->occupied;
        *br_injection = injection_point;
        SAFE(put_elem_in_RPN(stack_mach, 0, NULL, br_wrapper));
        // from here
        size_t br_inj = -1;
        SAFE(parse_logic(expr, stack_mach, &br_inj));
        // to here
        while (isspace(*(expr->curpointer))) ++(expr->curpointer);
        if (*(expr->curpointer) != ')') return E_UNBALANCED_LB;
        ++(expr->curpointer);
        SAFE(put_elem_in_RPN(stack_mach, 0, NULL, execute));
        inject_size(stack_mach, stack_mach->occupied - injection_point - sizeof(struct input_data), injection_point);
        while (isspace(*(expr->curpointer))) ++(expr->curpointer);
    } else if (isalpha(*(expr->curpointer))) {
        SAFE(parse_prog_call(expr, stack_mach));
    } else {
        printf("un_s = %c\n", *(expr->curpointer));
        return E_UNEXPECTED_SYMBOL;
    }
    return 0;
}

int parse_prog_call(Expression *expr, RPN *stack_mach) {
    int flag;
    int size = 0;
    unsigned char var[129];
    while (((*(expr->curpointer) != '|') && (*(expr->curpointer) != ';') && (*(expr->curpointer) != '>') &&
            (*(expr->curpointer) != '<') &&
            (*(expr->curpointer) != '\0') && (*(expr->curpointer) != '&') && (*(expr->curpointer) != '\n') &&
            (*(expr->curpointer) != ')')) &&
           (size < 128))
        var[size++] = *(expr->curpointer++);
    var[size++] = '\0';
    SAFE(put_elem_in_RPN(stack_mach, size, var, prog2stack));
    return 0;
}

int compute_expression(Expression *expr, int *res) {
    RPN stack_machine;
    int flag;
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    };
    size_t rpn_estimate = 8192;
    SAFE(RPN_init(&stack_machine, rpn_estimate));
    int ret = 0;
    size_t br_p = -1;
    if ((flag = parse_logic(expr, &stack_machine, &br_p)) != 0) {
        RPN_finalize(&stack_machine);
        return flag;
    }
    if (br_p == -1) SAFE(put_elem_in_RPN(&stack_machine, 0, NULL, execute));
    else
        SAFE(put_elem_in_RPN(&stack_machine, sizeof(br_p), &br_p, execute_brackets));
    // this realloc will never do anything because we demand a smaller chunk of memory so it will
    // always return the same pointer therefore there will never be an error
    stack_machine.data = realloc(stack_machine.data, stack_machine.occupied);
    stack_machine.data_size = stack_machine.occupied;
    while (isspace(*(expr->curpointer))) ++(expr->curpointer);

    if ((*(expr->curpointer) == '\0')) {
        if ((flag = RPN_compute(&stack_machine, &ret, sizeof(ret))) != 0) {
            RPN_finalize(&stack_machine);
            return flag;
        }
        *res = ret;
        // TODO: FIX THIS
        while (wait(NULL) > 1) {}
        RPN_finalize(&stack_machine);
        return 0;
    } else if (*(expr->curpointer) == ')') {
        RPN_finalize(&stack_machine);
        return E_UNBALANCED_RB;
    } else if (isalnum(*(expr->curpointer))) {
        RPN_finalize(&stack_machine);
        return E_MISSED_OPERATOR;
    }
    RPN_finalize(&stack_machine);
    printf("un_s = %c(%d)\n", *(expr->curpointer), *(expr->curpointer));
    return E_UNEXPECTED_SYMBOL;
}

int init_expression(Expression *expr, char *input) {
    expr->string_form = strdup(input);
    if (expr->string_form == NULL) return E_MEM_ALLOC;
    expr->curpointer = expr->string_form;
    return 0;
}

void finalize_expression(Expression *expr) {
    free(expr->string_form);
}
