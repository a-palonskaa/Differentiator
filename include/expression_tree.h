#ifndef EXPRESSION_TREE_H
#define EXPRESSION_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include "text_lib.h"

#define MAX_OP_LEN 10

typedef enum {
    NUM = 0,
    VAR = 1,
    OP  = 2,
    NIL = 3,
} type_t;

typedef enum {
    ADD    = 0,
    SUB    = 1,
    MUL    = 2,
    DIV    = 3,

    POW    = 4,
    LOG    = 5,
    LN     = 6,
    EXP    = 7,
    SIN    = 8,
    COS    = 9,
    TG     = 10,
    CTG    = 11,
    SH     = 12,
    CH     = 13,
    TH     = 14,
    CTH    = 15,
    ARCSIN = 16,
    ARCCOS = 17,
    ARCTG  = 18,
    ARCCTG = 19,
    ARCSH  = 20,
    ARCCH  = 21,
    ARCTH  = 22,
    ARCCTH = 23,
} op_t;

struct node_t{
    node_t* parent;
    node_t* left;
    node_t* right;

    type_t type;
    double value;
};

struct var_table_t {
    char name;
    double alias;
};

const var_table_t var_table[] = {{'x', 'x'},
                                 {'y', 'y'}};

typedef enum {
    NO_ERR     = 0,
    SYNTAX_ERR = 1,
} err_t;

typedef enum {
    RIGHT = 0,
    LEFT  = 1,
    ROOT  = 2,
} rel_t;

class exp_tree_t {
public:
    err_t init(FILE* data_file);
    void read_tree_r(text_t* text, size_t* index, node_t* parent, rel_t rel);
    void dtor(node_t* root);

    node_t* new_node(type_t type, double value, node_t* left, node_t* right, node_t* parent, rel_t rel);
    void delete_subtree_r(node_t* node);

    void set_dump_ostream(FILE* ostream);
    void print_preorder_();
    void print_inorder_();
    void print_preorder(node_t* node);

    void print_links(FILE* tree_file, node_t* node);
    void print_nodes(FILE* tree_file, node_t* node, size_t rank);
    void printf_tree_dot_file(FILE* tree_file, node_t* node);
    void dump(node_t* root);

    node_t* new_initial_node_r(text_t* text, node_t* parent, size_t* index);
    node_t* new_initial_node_infix(text_t* text, node_t* parent, size_t* index);
    int def_operator(char* op);
    node_t* root_;

    double calculate_value(double op_type, double val_l, double val_r);
    void calculate_expression_r(node_t* node);
    double calculate_expression(node_t* node);

    void print_to_tex(FILE* ostream, node_t* node);
    void print_tree_to_tex(FILE* ostream, node_t* root);

    int get_operator_precedence(int op);
    void print_inorder(FILE* ostream, node_t* node, int parent_precedence);

    node_t* copy_subtree(node_t* node);
    void differentiate_operation(FILE* ostream, node_t* node, node_t* op_node);
    node_t* differentiate(FILE* ostream, node_t* node);
    node_t* differentiate_expression(FILE* ostream);
    void calculations_optimization_r(node_t* node);

    void one_val_optimization(node_t* node, rel_t rel, rel_t parent_rel);
    void null_val__optimization(node_t* node, rel_t rel, rel_t parent_rel);
    void basic_operations_optimization_r(node_t* node, rel_t rel);

    bool is_var_present_r(node_t* node);
    void print_derivative_to_tex(FILE* ostream, node_t* node);
    void print_exp_to_tex(FILE* ostream, node_t* node);

// Grammar

    void SyntaxError(char* s, size_t p);
    node_t* GetN(char* s, size_t* p);
    node_t* GetE(char* s, size_t* p);
    node_t* GetT(char* s, size_t* p);
    node_t* GetP(char* s, size_t* p);
    node_t* GetG(char* s);
    node_t* GetV(char* s, size_t* p);
private:
    void print_operator(FILE* ostream, double value);
};



#endif /* EXPRESSION_TREE_H */
