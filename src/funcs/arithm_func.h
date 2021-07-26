#include "rec_desc.h"
int execute(Calculation_data *data);
int execute_nonblock(Calculation_data *data);
int prog2stack(Calculation_data *data);
int chg_in_stream(Calculation_data *data);
int chg_out_stream(Calculation_data *data);
int skip_exec(Calculation_data *data);
int logic_and(Calculation_data *data);
int logic_or(Calculation_data *data);
int br_wrapper(Calculation_data *data);
int execute_brackets(Calculation_data *data);
int execute_brackets_nonblock(Calculation_data *data);