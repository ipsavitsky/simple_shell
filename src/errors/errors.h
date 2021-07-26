/**
 * enum of all errors possible in rec_desc and all of its dependencies
 */
enum {
    E_MEM_ALLOC = 1,          ///< error in allocating memory
    E_OVERFLOW = 2,           ///< error in dynamic memory overflow
    E_UNDERFLOW = 3,          ///< error in dynamic memory underflow
    E_UNEXPECTED_SYMBOL = 4,  ///< error in case of unexpected symbol
    E_ZERO_DIVISION = 5,      ///< arithmetic exception in case of division by zero
    E_UNKNOWN_VAR = 6,        ///< error in case variable lookup
    E_UNBALANCED_LB = 7,      ///< error in case of an unbalanced bracket
    E_UNBALANCED_RB = 8,      ///< error in case of an unbalanced bracket
    E_MISSED_OPERATOR = 9,    ///< error in case of missed operator
    E_UNKNOWN_EXEC = 10,
    E_OPENING_FILE = 11,
    E_FILE_OP = 12,
    E_PIPE = 13,
    E_REDERECT = 14
};

/**
 * print an error message according to the error code
 * \param err error code
 */
void err_print(int err);
