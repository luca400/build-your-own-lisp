#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>
#include "mpc.h"

// Lval stands for lisp value
typedef struct {
    int type;
    long num;
    int err;
} lval;

// Create Enumaration of possible Lval types
enum { LVAL_NUMBER, LVAL_ERR };

// Create Enumaration of possible error types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// Create a new number type lval
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUMBER;
    v.num = x;
    return v;
}

// Create a new error type lval
lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch (v.type){
        case LVAL_NUMBER: printf("%li", v.num); break;
        case LVAL_ERR:
            if(v.err == LERR_DIV_ZERO)
                printf("ERROR: Divizion by zero is not possible");
            if(v.err == LERR_BAD_OP)
                printf("ERROR: Invalid operator");
            if(v.err == LERR_BAD_NUM)
                printf("ERROR: Invalid number"); 
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

//TODO: Implement remainder operator ( % )
lval eval_op(lval x, char *op, lval y) {
    if(x.type == LVAL_ERR) return x;
    if(x.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) {return lval_num(x.num + y.num);}
    if (strcmp(op, "-") == 0) {return lval_num(x.num - y.num);}
    if (strcmp(op, "*") == 0) {return lval_num(x.num * y.num);}
    if (strcmp(op, "/") == 0) {
        return y.num == 0 
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);
    } 

    return lval_err(LERR_BAD_OP);
} 

lval eval(mpc_ast_t *t){
    /* If tagged as number return it directly. */
    if (strstr(t->tag, "number")) {
        // Check if there is some error in conversion;
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /* The operator is always second child. */
    char* op = t->children[1]->contents;

    /* We store the third child in `x` */
    lval x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int main(int argc, char** argv) {

    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                     \
        number   : /-?[0-9]+/ ;                             \
        operator : '+' | '-' | '*' | '/' ;                  \
        expr     : <number> | '(' <operator> <expr>+ ')' ;  \
        lispy    : /^/ <operator> <expr>+ /$/ ;             \
      ",
      Number, Operator, Expr, Lispy);
            
    puts("Lispy version 0.0.0.0.0.1");
    puts("Press ctrl+c to Exit\n");

    while(1) {
        char *input = readline("lispy> ");

        add_history(input);
        
        /* Attempt to Parse the user Input */
        mpc_result_t r;
        
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
          
          /* On Success Print the AST (Abstract syntax tree) */
          lval result = eval(r.output);
          lval_println(result);
          mpc_ast_delete(r.output);
        
        } else {
          /* Otherwise Print the Error */
          mpc_err_print(r.error);
          mpc_err_delete(r.error);
        }

    }
    
    /* Undefine and Delete our Parsers */
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}


