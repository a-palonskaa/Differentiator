#ifndef EXPRESSION_TREE_H
#define EXPRESSION_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include "text_lib.h"

#define MAX_OP_LEN 10
#define MAX_NAME_LEN 11

typedef enum {
    NUM = 0,
    VAR = 1,
    OP  = 2,
} type_t;

typedef enum {
    POISON = -1,
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

    BRACKET_OPEN  = 24,
    BRACKET_CLOSE = 25,
    EOT           = 26,
} op_t;

typedef struct {
    char name[MAX_NAME_LEN];
}  name_t;

struct node_t {
    node_t* parent;
    node_t* left;
    node_t* right;

    type_t type;
    double value;
};

typedef enum {
    NO_ERR             = 0,
    SYNTAX_ERR         = 1,
    MEM_ALLOC_ERR      = 2,
    NUM_INVAR_ERR      = 3,
    UN_OP_INVAR_ERR    = 4,
    BIN_OP_INVAR_ERR   = 5,
    SUB_SYNTAX_ERR     = 6,
    ADD_SYNTAX_ERR     = 7,
    INVALID_ROOT_ERR   = 8,
    CYCLIC_LINKING_ERR = 9,
} err_t;

typedef enum {
    RIGHT = 0,
    LEFT  = 1,
    ROOT  = 2,
} rel_t;

typedef struct {
    char name[MAX_NAME_LEN];
    op_t code;
} func_name_table_t;

const func_name_table_t func_name_table[] = {
    { "+",      ADD},
    { "-",      SUB},
    { "/",      DIV},
    { "*",      MUL},
    { "^",      POW},
    { "log",    LOG},
    { "ln",     LN},
    { "exp",    EXP},
    { "arcsin", ARCSIN},
    { "arccos", ARCCOS},
    { "arctg",  ARCTG},
    { "arcctg", ARCCTG},
    { "arcch",  ARCCH},
    { "arcsh",  ARCSH},
    { "arcth",  ARCTH},
    { "arccth", ARCCTH},
    { "sin",    SIN},
    { "cos",    COS},
    { "tg",     TG},
    { "ctg",    CTG},
    { "sh",     SH},
    { "ch",     CH},
    { "cth",    CTH},
    { "th",     TH}};

const size_t func_name_table_len = sizeof(func_name_table) / sizeof(func_name_table[0]);

class exp_tree_t {
public:
    err_t init(FILE* data_file);
    void dtor();
    void delete_tree(node_t* root);

    void set_dump_ostream(FILE* ostream);
    void print_preorder_();
    void print_inorder_();
    void print_preorder(node_t* node);

    void print_links(FILE* tree_file, node_t* node);
    void print_nodes(FILE* tree_file, node_t* node, size_t rank);
    void printf_tree_dot_file(FILE* tree_file, node_t* node);
    void dump(node_t* root);
    void dump_tree();

    node_t* new_initial_node_r(text_t* text, node_t* parent, size_t* index);
    node_t* new_initial_node_infix(text_t* text, node_t* parent, size_t* index);
    int def_operator(char* op);

    double calculate_value(double op_type, node_t* node_l, node_t* node_r);
    void calculate_expression_r(node_t* node);
    double calculate_expression(node_t* node);

    void print_tree_to_tex(FILE* ostream, node_t* root);
    void print_exp_to_tex(FILE* ostream, node_t* node);

    node_t* differentiate_expression(FILE* ostream);

    node_t* optimize(node_t* node);

    err_t verify(node_t* root);
private:
    void add_parents_rel_r(node_t* node, node_t* parent);
    node_t* new_node(type_t type, double value, node_t* left, node_t* right, node_t* parent, rel_t rel);
    void delete_subtree_r(node_t* node);

    int get_operator_precedence(int op);
    void print_to_tex(FILE* ostream, node_t* node);
    void print_operator(FILE* ostream, double value);
    void print_inorder(FILE* ostream, node_t* node, int parent_precedence);

    bool calculations_optimization_r(node_t* node);
    node_t* basic_operations_optimization_r(node_t* node, rel_t rel, bool* flag);
    node_t* one_val_optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag);
    node_t* null_val__optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag);

    bool is_var_present_r(node_t* node);
    void print_derivative_to_tex(FILE* ostream, node_t* node);

    node_t* copy_subtree(node_t* node);
    void differentiate_operation(FILE* ostream, node_t* node, node_t* op_node);
    node_t* differentiate(FILE* ostream, node_t* node);

// Grammar

    void syntax_error(size_t p, const char* func, size_t line);

    node_t* get_g();
    node_t* get_n(size_t* p);
    node_t* get_e(size_t* p);
    node_t* get_t(size_t* p);
    node_t* get_p(size_t* p);
    node_t* get_v(size_t* p);
    node_t* get_op(size_t* p);

    node_t* token_init(text_t* text);
    node_t* link_tokens();
    void tokenize_text(text_t* text);
    void parse_identificator(text_t* text, size_t* ip, node_t* node);
    op_t is_operator(char* name);
    bool is_unary(double value);
    void parse_number(text_t* text, size_t* ip, node_t* node);
    void initialize_op_node(node_t* node, double val);
    void print_tokens_array();

    double find_name_in_nametable(char* name);
    double add_name_to_nametable(char* name);
    double index_in_nametable(char* name);
    void print_var_nametable();

    bool is_tree_acyclic_r(node_t* node, node_t* parent);
    err_t check_op_type_invariants_r(node_t* node);
private:
    name_t var_nametable_[100];
    size_t var_nametable_size_{0};
    node_t* root_;
    node_t* tokens_{nullptr};
    size_t tokens_array_size_{0};
};

#endif /* EXPRESSION_TREE_H */
