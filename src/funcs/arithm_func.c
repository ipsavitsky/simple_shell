#include "arithm_func.h"

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SAFE(call)                           \
    do {                                     \
        if ((flag = call) != 0) return flag; \
    } while (0)

int prog2stack(Calculation_data *data) {  return stack_push(data->stack, data->elem, data->size); }
int chg_in_stream(Calculation_data *data) {  return stack_push(data->stack, data->elem, data->size); }
int chg_out_stream(Calculation_data *data) { return stack_push(data->stack, data->elem, data->size); }

int logic_and(Calculation_data *data){
    int flag, val;
    SAFE(stack_pop(data->stack, &val, sizeof(val)));
    if(val != 0){
        *(data->pos_jmp) = *((size_t *)data->elem);
    }
    return 0;
}

int logic_or(Calculation_data *data){
    int flag, val;
    SAFE(stack_pop(data->stack, &val, sizeof(val)));
    if(val == 0){
        *(data->pos_jmp) = *((size_t *)data->elem);
    }
    return 0;
}

int br_wrapper(Calculation_data *data){
    return 0;
}

int skip_exec(Calculation_data *data) {
    int useless, flag;
    SAFE(stack_pop(data->stack, &useless, sizeof(useless)));
    return 0;
}

// move oldfd to newfd
int update_fd(int oldfd, int newfd){
    if (oldfd != newfd){
        if (dup2(oldfd, newfd) != -1) close(oldfd);     // update_fd successful
        else return E_REDERECT;
    }
    return 0;
}

int execute_brackets(Calculation_data *data){
    void *br_point = &(((char *)data->expression->data)[*(size_t *)(data->elem)]);
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    }br_point_dat = *((struct input_data *)br_point);
    RPN sub_rpn = {.data = (char *)br_point + sizeof(struct input_data), .data_size = br_point_dat.size, .occupied = br_point_dat.size};
    pid_t pid;
    int ret = 0, flag, stat;
    long out_desc, in_desc;
    SAFE(stack_pop(data->stack, &in_desc, sizeof(in_desc)));
    SAFE(stack_pop(data->stack, &out_desc, sizeof(out_desc)));
    if((pid = fork()) == 0){
        SAFE(update_fd(out_desc, STDOUT_FILENO));
        SAFE(update_fd(in_desc, STDIN_FILENO));
        RPN_compute(&sub_rpn, &ret, sizeof(ret));
        exit(0);
    }
    while (1) {
        waitpid(pid, &stat, 0);
        if (WIFEXITED(stat) == 1) {
            int status = WEXITSTATUS(stat);
            SAFE(stack_push(data->stack, &status, sizeof(status)));
            break;
        }
    }
    if(in_desc != 0) close(in_desc);
    if(out_desc != 1) close(out_desc);
    return 0;
}

int execute_brackets_nonblock(Calculation_data *data){
    void *br_point = &(((char *)data->expression->data)[*(size_t *)(data->elem)]);
    struct input_data {
        Size_elem size;
        Calculate_elem f;
    }br_point_dat = *((struct input_data *)br_point);
    RPN sub_rpn = {.data = (char *)br_point + sizeof(struct input_data), .data_size = br_point_dat.size, .occupied = br_point_dat.size};
    pid_t pid;
    int ret = 0, flag;
    long out_desc, in_desc;
    SAFE(stack_pop(data->stack, &in_desc, sizeof(in_desc)));
    SAFE(stack_pop(data->stack, &out_desc, sizeof(out_desc)));
    if((pid = fork()) == 0){
        SAFE(update_fd(out_desc, STDOUT_FILENO));
        SAFE(update_fd(in_desc, STDIN_FILENO));
        RPN_compute(&sub_rpn, &ret, sizeof(ret));
        exit(0);
    }
    if(in_desc != 0) close(in_desc);
    if(out_desc != 1) close(out_desc);
    return 0;
}


int execute(Calculation_data *data) {
    pid_t pid;
    int stat, flag;
    char *curpars;
    // FIXME: this is gross
    char *args[128];
    char full_call[128];
    long out_desc, in_desc;
    SAFE(stack_pop(data->stack, &in_desc, sizeof(in_desc)));
    SAFE(stack_pop(data->stack, &out_desc, sizeof(out_desc)));
    SAFE(stack_pop(data->stack, full_call, sizeof(full_call)));
    int arg_cnt = 0;
    curpars = full_call;
    while (1) {
        if (*curpars == '\0') break;
        args[arg_cnt++] = curpars;
        while ((!isspace(*curpars)) && (*curpars != '\0')) curpars++;
        if (*curpars == '\0') break;
        *curpars = '\0';
        curpars++;
        while (isspace(*curpars)) curpars++;
    }
    args[arg_cnt] = NULL;
    if ((pid = fork()) == 0) {
        // signal(SIGTERM, kill_myself);
        // fprintf(stderr, "spawned process: %d\n", getpid());
        SAFE(update_fd(out_desc, STDOUT_FILENO));
        SAFE(update_fd(in_desc, STDIN_FILENO));
        execvp(args[0], args);
        // perror("execve()");
        return E_UNKNOWN_EXEC;
    }
    // while(waitpid(-1, &stat, WNOHANG) > 1){}
    while (1) {
        waitpid(pid, &stat, 0);
        if (WIFEXITED(stat) == 1) {
            int status = WEXITSTATUS(stat);
            SAFE(stack_push(data->stack, &status, sizeof(status)));
            break;
        }
    }
    if(in_desc != 0) close(in_desc);
    if(out_desc != 1) close(out_desc);
    return 0;
}

int execute_nonblock(Calculation_data *data) {
    pid_t pid;
    int flag;
    char *curpars;
    char *args[15];
    char full_call[128];
    long out_desc, in_desc;
    SAFE(stack_pop(data->stack, &in_desc, sizeof(in_desc)));
    SAFE(stack_pop(data->stack, &out_desc, sizeof(out_desc)));
    SAFE(stack_pop(data->stack, full_call, sizeof(full_call)));
    int arg_cnt = 0;
    curpars = full_call;
    while (1) {
        if (*curpars == '\0') break;
        args[arg_cnt++] = curpars;
        while ((!isspace(*curpars)) && (*curpars != '\0')) curpars++;
        if (*curpars == '\0') break;
        *curpars = '\0';
        curpars++;
        while (isspace(*curpars)) curpars++;
    }
    args[arg_cnt] = NULL;
    if ((pid = fork()) == 0) {
        update_fd(out_desc, STDOUT_FILENO);
        update_fd(in_desc, STDIN_FILENO);
        execvp(args[0], args);
        return E_UNKNOWN_EXEC;
    }
    if(in_desc != 0) close(in_desc);
    if(out_desc != 1) close(out_desc);
    return 0;
}