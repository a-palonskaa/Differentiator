#include <stdio.h>
#include <stdlib.h>
#include "expression_tree.h"
#include "logger.h"

void exp_tree_t::syntax_error(size_t p, const char* func, size_t line) {
    LOG(ERROR, "Syntax error p = %zu, type = %d(val = %f) func: %s (%zu)\n",
               p, tokens_[p].type, tokens_[p].value, func, line);
    exit(0);
}

node_t* exp_tree_t::get_e(size_t* p) {
    node_t* val = get_t(p);
    while (tokens_[*p].type == OP &&
          ((int) tokens_[*p].value == ADD ||
           (int) tokens_[*p].value == SUB)) {
        size_t op_p = (*p)++;
        node_t* val2 = get_t(p);

        tokens_[op_p].left = val;
        tokens_[op_p].right = val2;
        val = &tokens_[op_p];
    }
    return val;
}

node_t* exp_tree_t::get_t(size_t* p) {
    node_t* val = get_p(p);
    while (tokens_[*p].type == OP &&
          ((int) tokens_[*p].value == MUL ||
           (int) tokens_[*p].value == DIV)) {
        size_t op_p = (*p)++;
        node_t* val2 = get_p(p);

        tokens_[op_p].left = val;
        tokens_[op_p].right = val2;
        val = &tokens_[op_p];
    }
    return val;
}

node_t* exp_tree_t::get_p(size_t* p) {
    node_t* val1 = nullptr;

    if (tokens_[*p].type == OP && (int) tokens_[*p].value == SUB) {
        size_t old_p = *p;
        (*p)++;
        tokens_[old_p].right = get_p(p);
        tokens_[old_p].left = nullptr;
        val1 = &tokens_[old_p];
    }
    else if (tokens_[*p].type == OP && (int) tokens_[*p].value == ADD) {
        size_t old_p = *p;
        (*p)++;
        tokens_[old_p].right = get_p(p);
        tokens_[old_p].left = nullptr;
        val1 = &tokens_[old_p];
    }
    else if (tokens_[*p].type == OP && (int) tokens_[*p].value == BRACKET_OPEN) {
        (*p)++;
        node_t* val = get_e(p);

        if (!(tokens_[*p].type == OP && (int) tokens_[*p].value == BRACKET_CLOSE)) {
            syntax_error(*p, __func__, __LINE__);
            delete_subtree_r(val);
            return nullptr;
        }

        (*p)++;
        val1 = val;
    }
    else if ((val1 = get_n(p)) != nullptr ||
             (val1 = get_v(p)) != nullptr ||
             (val1 = get_op(p)) != nullptr) {
        ;
    }
    else {
        syntax_error(*p, __func__, __LINE__);
        return nullptr;
    }

    if (tokens_[*p].type == OP && (int) tokens_[*p].value == POW) {
        (*p)++;
        val1 = new_node(OP, POW, val1, get_p(p), nullptr, ROOT);
    }
    return val1;
}

node_t* exp_tree_t::get_g() {
    size_t p = 0;
    node_t* val = get_e(&p);

    if (tokens_[p].type == OP && (int) tokens_[p].value != EOT) {
        syntax_error(p, __func__, __LINE__);
        delete_subtree_r(val);
        val = nullptr;
    }
    return val;
}

node_t* exp_tree_t::get_v(size_t* p) {
    size_t old_p = *p;

    if (tokens_[*p].type == VAR) {
        (*p)++;
    }
    return (old_p == *p) ? nullptr : &tokens_[old_p];
}

node_t* exp_tree_t::get_n(size_t* p) {
    size_t old_p = *p;

    if (tokens_[*p].type == NUM) {
        (*p)++;
    }
    return (old_p == *p) ? nullptr : &tokens_[old_p];
}

node_t* exp_tree_t::get_op(size_t* p) {
    size_t old_p = *p;

    if (tokens_[*p].type == OP &&
       ((int) tokens_[*p].value == SIN    ||
        (int) tokens_[*p].value == COS    ||
        (int) tokens_[*p].value == TG     ||
        (int) tokens_[*p].value == CTG    ||
        (int) tokens_[*p].value == SH     ||
        (int) tokens_[*p].value == CH     ||
        (int) tokens_[*p].value == TH     ||
        (int) tokens_[*p].value == CTH    ||
        (int) tokens_[*p].value == ARCSIN ||
        (int) tokens_[*p].value == ARCCOS ||
        (int) tokens_[*p].value == ARCTG  ||
        (int) tokens_[*p].value == ARCCTG ||
        (int) tokens_[*p].value == ARCSH  ||
        (int) tokens_[*p].value == ARCCH  ||
        (int) tokens_[*p].value == ARCTH  ||
        (int) tokens_[*p].value == ARCCTH ||
        (int) tokens_[*p].value == EXP    ||
        (int) tokens_[*p].value == LN)) {
        (*p)++;
        tokens_[old_p].left = get_p(p);
    }
    return (old_p == *p) ? nullptr : &tokens_[old_p];
}
