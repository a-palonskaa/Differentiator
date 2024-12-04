#include <stdio.h>
#include <stdlib.h>
#include "expression_tree.h"

void exp_tree_t::SyntaxError(char* s, size_t p) {
    printf("Syntax error p = %zu, %c(%d)\n", p, s[p], s[p]);
    exit(0);
}

node_t* exp_tree_t::GetE(char* s, size_t* p) {
    node_t* val = GetT(s, p);
    while (s[*p] == '+' || s[*p] == '-') {
        op_t op = (s[*p] == '+') ? ADD : SUB;
        (*p)++;
        node_t* val2 = GetT(s, p);
        val = new_node(OP, op, val, val2, nullptr, ROOT);
    }
    return val;
}

node_t* exp_tree_t::GetT(char* s, size_t* p) {
    node_t* val = GetP(s, p);
    while (s[*p] == '*' || s[*p] == '/') {
        op_t op = (s[*p] == '*') ? MUL : DIV;
        (*p)++;
        node_t* val2 = GetP(s, p);
        val = new_node(OP, op, val, val2, nullptr, ROOT);
    }
    return val;
}

node_t* exp_tree_t::GetP(char* s, size_t* p) {
    node_t* val1 = nullptr;
    if (s[*p] == '(') {
        (*p)++;
        node_t* val = GetE(s, p);
        if (s[*p] != ')')
            SyntaxError(s, *p);
        (*p)++;
        return val;
    }
    else if ((val1 = GetN(s, p)) != nullptr) {
        return val1;
    }
    else if ((val1 = GetV(s, p)) != nullptr) {
        return val1;
    }
    else {
        SyntaxError(s, *p);
        return nullptr;
    }
}

node_t* exp_tree_t::GetG(char* s) {
    size_t p = 0;
    node_t* val = GetE(s, &p);
    if (s[p] != '$')
        SyntaxError(s, p);
    p++;
    return val;
}

node_t* exp_tree_t::GetV(char* s, size_t* p) {
    size_t oldp = *p;
    if (s[*p] == 'x') {
        (*p)++;
    }
    return (oldp == *p) ? nullptr : new_node(VAR, 'x', nullptr, nullptr, nullptr, ROOT);
}

node_t* exp_tree_t::GetN(char* s, size_t* p) {
    int val = 0;
    size_t oldp = *p;
    while ('0' <= s[*p] && s[*p] <= '9') {
        val = val * 10 + s[*p] - '0';
        (*p)++;
    }
    return (oldp == *p) ? nullptr : new_node(NUM, val, nullptr, nullptr, nullptr, ROOT);
}
