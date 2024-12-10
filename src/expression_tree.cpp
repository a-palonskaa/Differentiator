#include <assert.h>
#include <string.h>
#include <errno.h>
#include "logger.h"
#include "expression_tree.h"

//===================================CTOR/DTOR===================================================

void exp_tree_t::dtor() {
    free(tokens_);
    tokens_ = nullptr;
}

void exp_tree_t::delete_tree(node_t* root) {
    delete_subtree_r(root);
}

void exp_tree_t::delete_subtree_r(node_t* node) {
    if (node == nullptr) {
        return;
    }

    if (node->left != nullptr) {
        delete_subtree_r(node->left);
    }
    if (node->right != nullptr) {
        delete_subtree_r(node->right);
    }

    free(node);
    node = nullptr;
}

node_t* exp_tree_t::new_node(type_t type, double value, node_t* left, node_t* right, node_t* parent, rel_t rel) {
    node_t* new_node = (node_t*) calloc(sizeof(node_t), sizeof(char));
    if (new_node == nullptr) {
        return nullptr;
    }

    new_node->type = type;
    new_node->value = value;

    new_node->parent = parent;
    new_node->right = right;
    if (right != nullptr) right->parent = new_node;

    new_node->left = left;
    if (left != nullptr) left->parent = new_node;

    if (parent != nullptr) {
        switch (rel) {
            case RIGHT:
                parent->right = new_node;
                break;
            case LEFT:
                parent->left = new_node;
                break;
            case ROOT:
                break;
            default:
                break;
        }
    }
    return new_node;
}

//==========================================INIT==================================================

err_t exp_tree_t::init(FILE* data_file) {
    assert(data_file != nullptr);

    text_t text = {};
    if (text_ctor(&text, data_file) != TEXT_NO_ERRORS) {
        LOG(ERROR, "Failed to read text\n");
        return SYNTAX_ERR;
    }

    root_ = token_init(&text);

    text_dtor(&text);
    return NO_ERR;
}

node_t* exp_tree_t::new_initial_node_r(text_t* text, node_t* parent, size_t* index) {
    assert(text != nullptr);
    assert(index != nullptr);

    double num = 0;
    char symb[2] = "";
    char exp[2] = "";
    char op[MAX_OP_LEN] = "";
    int read_characters = 0;

    node_t* current_node = (node_t*) calloc(sizeof(node_t), sizeof(char));
    if (current_node == nullptr) {
       LOG(ERROR, "Memory allocation error\n" STRERROR(errno));
        return nullptr;
    }

    int meow = 0;
    if (sscanf((char*) &text->symbols[*index], "%lf%n", &num, &read_characters) != 0) {
        current_node->type = NUM;
        current_node->value = num;
    }
    else if ((sscanf((char*) &text->symbols[*index], "%1[a-z]%n", symb, &meow) != 0) &&
            (sscanf((char*) &text->symbols[*index + 1], "%1[()]%n", exp, &meow) != 0))  {
        current_node->type = VAR;
        current_node->value = int(symb[0]);
        read_characters = 1;
    }
    else if (sscanf((char*) &text->symbols[*index], "%9[^()]", op) != 0) {
        read_characters = (int) strnlen(op, MAX_OP_LEN);
        current_node->type = OP;
        current_node->value = def_operator(op);
    }
    else if (sscanf((char*) &text->symbols[*index], "%1[)]", symb) != 0) {
        read_characters = 0;
        current_node->type = NIL;
        current_node->value = NULL;
    }

    *index += (size_t) read_characters;

    current_node->parent = parent;

    size_t i = *index;
    for (; i < text->symbols_amount; i++) {
        if (text->symbols[i] == ')') {
            current_node->left = nullptr;
            current_node->right = nullptr;

            *index = ++i;
            return current_node;
        }
        if (text->symbols[i] == '(') {
            *index = ++i;
            current_node->left = new_initial_node_r(text, current_node, index);
            break;
        }
    }

    for (i = *index; i < text->symbols_amount; i++) {
        if (text->symbols[i] == '(') {
            *index = ++i;
            current_node->right = new_initial_node_r(text, current_node, index);
            break;
        }
    }
    return current_node;
}

node_t* exp_tree_t::new_initial_node_infix(text_t* text, node_t* parent, size_t* index) {
    (void) text;
    (void) parent;
    (void) index;
return nullptr;
}

//===================================CALCULATE================================================

double exp_tree_t::calculate_expression(node_t* node) {
    calculate_expression_r(node);
    return node->value;
};

void exp_tree_t::calculate_expression_r(node_t* node) {
    if (node == nullptr) {
        return;
    }

    if (node->left != nullptr) {
        calculate_expression_r(node->left);
    }

    if (node->right != nullptr) {
        calculate_expression_r(node->right);
    }

    if (node->type == OP && node->left != nullptr && node->right != nullptr) {
        double val_l = 0;
        double val_r = 0;

        if (node->left->type == VAR) {
            printf("Enter the value of '%s' variable: ", "x");
            scanf("%lf", &val_l);
        } else {
            val_l = node->left->value;
        }

        if (node->right->type == VAR) {
            printf("Enter the value of '%s' variable: ", "x");
            scanf("%lf", &val_r);
        } else {
            val_r = node->right->value;
        }

        node->value = calculate_value(node->value, val_l, val_r);

        free(node->left);
        free(node->right);

        node->left = nullptr;
        node->right = nullptr;
        node->type = NUM;
    }
}

double exp_tree_t::calculate_value(double op_type, double val_l, double val_r) {
    switch ((int) op_type) {
        case ADD:
            return val_l + val_r;
            break;
        case SUB:
            return val_l - val_r;
            break;
        case MUL:
            return val_l * val_r;
            break;
        case DIV:
            return val_l / val_r;
            break;
        default:
            LOG(ERROR, "Undefined operation %d(%lf)\n", (int) op_type, op_type);
            return NULL;
            break;
    }

}

//===================================DIFFERENTIATE================================================

node_t* exp_tree_t::differentiate_expression(FILE* ostream) {
    return differentiate(ostream, root_);
}

node_t* exp_tree_t::differentiate(FILE* ostream, node_t* node) {
    if (node == nullptr) return nullptr;

    node_t* diff_root = new_node(OP, 0, nullptr, nullptr, nullptr, ROOT);
    if (diff_root == nullptr) return nullptr;

    switch (node->type) {
        case VAR: {
            diff_root->type = NUM;
            diff_root->value = 1;
            fprintf(ostream, "Initial expression: \n\n");
            print_exp_to_tex(ostream, node);
            fprintf(ostream, "We get that the derivative of variable: \n\n");
            print_exp_to_tex(ostream, diff_root);
            break;
        }
        case NUM: {
            diff_root->type = NUM;
            diff_root->value = 0;
            fprintf(ostream, "Initial expression: \n\n");
            print_exp_to_tex(ostream, node);
            fprintf(ostream, "We get that the derivative of const is: \n\n");
            print_exp_to_tex(ostream, diff_root);
            break;
        }
        case OP: {
            differentiate_operation(ostream, node, diff_root);
            break;
        }
        case NIL: {
            free(diff_root);
            return nullptr;
        }
        default: {
            break;
        }
    }

    return diff_root;
}

void exp_tree_t::differentiate_operation(FILE* ostream, node_t* node, node_t* op_node) {
    if (op_node == nullptr) return;
    if (node == nullptr) return;
//STUB - DIV
    switch ((int) node->value) {
        case ADD:
            op_node->value = ADD;

            op_node->right = differentiate(ostream, node->right);
            op_node->right->parent = op_node;
            op_node->left = differentiate(ostream, node->left);
            op_node->left->parent = op_node;
            break;
        case SUB:
            op_node->value = SUB;

            op_node->right = differentiate(ostream, node->right);
            op_node->right->parent = op_node;
            op_node->left = differentiate(ostream, node->left);
            op_node->left->parent = op_node;
            break;
        case MUL:
            op_node->value = ADD;
            if (new_node(OP, MUL, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            if (new_node(OP, MUL, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            op_node->left->right = copy_subtree(node->right);
            if (op_node->left->right == nullptr) return;
            op_node->left->right->parent = op_node->left;

            op_node->right->left = copy_subtree(node->left);
            op_node->right->left->parent = op_node->right;
            if (op_node->right->left == nullptr) return;

            op_node->left->left = differentiate(ostream, node->left);
            if (op_node->left->left == nullptr) return;
            op_node->left->left->parent = op_node->left;

            op_node->right->right = differentiate(ostream, node->right);
            if (op_node->right->right == nullptr) return;
            op_node->right->right->parent = op_node->right;

            break;
        case DIV:
            op_node->value = SUB;

            if (new_node(OP, DIV, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            op_node->left->left = differentiate(ostream, node->left);
            if (op_node->left->left == nullptr) return;
            op_node->left->left->parent = op_node->left;

            op_node->left->right = copy_subtree(node->right);
            op_node->left->right->parent = op_node->left;

            if (new_node(OP, DIV, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            if (new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;
            if (new_node(NUM, 2, nullptr, nullptr, op_node->right->right, RIGHT) == nullptr) return;

            op_node->right->right->left = copy_subtree(node->right);
            op_node->right->right->left->parent = op_node->right->right;

            if (new_node(OP, MUL, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            op_node->right->left->left = differentiate(ostream, node->right);
            op_node->right->left->left->parent = op_node->right->left;

            op_node->right->left->right = copy_subtree(node->left);
            op_node->right->left->right->parent = op_node->right->left;
            break;
        case LOG:
            op_node->value = MUL;

            if (new_node(OP, DIV, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            op_node->left = differentiate(ostream, node->right);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(NUM, 1, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            if (new_node(OP, MUL, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

            if (new_node(OP, LN, nullptr, nullptr, op_node->right->right, LEFT) == nullptr) return;

            if (new_node(NIL, NULL, nullptr, nullptr, op_node->right->right->left, LEFT) == nullptr) return;

            op_node->right->right->right = copy_subtree(node->right);
            if (op_node->right->right->right == nullptr) return;
            op_node->right->right->right->parent = op_node->right->right;

            op_node->right->right->left->left = copy_subtree(node->left);
            if (op_node->right->right->left->left == nullptr) return;
            op_node->right->right->left->left->parent = op_node->right->right->left;
            break;
        case LN:
            op_node->value = MUL;

            if (new_node(OP, DIV, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            op_node->right = differentiate(ostream, node->left);
            if (op_node->right == nullptr) return;
            op_node->right->parent = op_node;

            if (new_node(NUM, 1, nullptr, nullptr, op_node->left, LEFT) == nullptr) return;

            op_node->left->right = copy_subtree(node->left);
            if (op_node->left->right == nullptr) return;
            op_node->left->right->parent = op_node->left; //FIXME - add container
            break;
        case EXP:
            op_node->value = MUL;

            if (new_node(OP, EXP, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(NIL, NULL, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            op_node->right->left = copy_subtree(node->left);
            if (op_node->right->left == nullptr) return;
            op_node->right->left->parent = op_node->right;
            break;
        case SIN:
            op_node->value = MUL;

            if (new_node(OP, COS, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            op_node->right->left = copy_subtree(node->left);
            if (op_node->right->left == nullptr) return;
            op_node->right->left->parent = op_node->right;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;
            break;
        case COS:
            op_node->value = MUL;

            op_node->right = new_node(OP, MUL, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            op_node->right->left = new_node(NUM, -1, nullptr, nullptr, op_node->right, LEFT);
            if (op_node->right->left == nullptr) return;

            op_node->right->right = new_node(OP, SIN, nullptr, nullptr, op_node->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left = copy_subtree(node->left);
            if (op_node->right->right->left == nullptr) return;
            op_node->right->right->left->parent = op_node->right->right;
            break;
        case TG:
            op_node->value = MUL;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            op_node->right = new_node(OP, DIV, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->right->left = new_node(NUM, 1, nullptr, nullptr, op_node->right, LEFT);
            if (op_node->right->left == nullptr) return;

            op_node->right->right = new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->right = new_node(NUM, 2, nullptr, nullptr, op_node->right->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left = new_node(OP, COS, nullptr, nullptr, op_node->right->right, LEFT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left->right = new_node(NIL, NULL, nullptr, nullptr, op_node->right->right->left, RIGHT);
            if (op_node->right->right->left->right == nullptr) return;

            op_node->right->right->left->left = copy_subtree(node->left);
            if (op_node->right->right->left->left == nullptr) return;
            op_node->right->right->left->left->parent = op_node->right->right->left;
            break;
        case CTG:
            op_node->value = MUL;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            op_node->right = new_node(OP, DIV, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->right->left = new_node(NUM, -1, nullptr, nullptr, op_node->right, LEFT);
            if (op_node->right->left == nullptr) return;

            op_node->right->right = new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->right = new_node(NUM, 2, nullptr, nullptr, op_node->right->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left = new_node(OP, SIN, nullptr, nullptr, op_node->right->right, LEFT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left->right = new_node(NIL, NULL, nullptr, nullptr, op_node->right->right->left, RIGHT);
            if (op_node->right->right->left->right == nullptr) return;

            op_node->right->right->left->left = copy_subtree(node->left);
            if (op_node->right->right->left->left == nullptr) return;
            op_node->right->right->left->parent = op_node->right->right->left;
            break;
        case SH:
            op_node->value = MUL;

            op_node->right = new_node(OP, CH, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->right->left = copy_subtree(node->left);
            if (op_node->right->left == nullptr) return;
            op_node->right->left->parent = op_node->right;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left  == nullptr) return;
            op_node->left->parent = op_node;
            break;
        case CH:
            op_node->value = MUL;

            op_node->right = new_node(OP, SH, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->right->left = copy_subtree(node->left);
            if (op_node->right->left == nullptr) return;
            op_node->right->left->parent = op_node->right;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left  == nullptr) return;
            op_node->left->parent = op_node;
            break;
        case TH:
            op_node->value = MUL;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            op_node->right = new_node(OP, DIV, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->right->left = new_node(NUM, 1, nullptr, nullptr, op_node->right, LEFT);
            if (op_node->right->left == nullptr) return;

            op_node->right->right = new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->right = new_node(NUM, 2, nullptr, nullptr, op_node->right->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left = new_node(OP, CH, nullptr, nullptr, op_node->right->right, LEFT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left->right = new_node(NIL, NULL, nullptr, nullptr, op_node->right->right->left, RIGHT);
            if (op_node->right->right->left->right == nullptr) return;

            op_node->right->right->left->left = copy_subtree(node->left);
            if (op_node->right->right->left->left) return;
            op_node->right->right->left->left->parent = op_node->right->right->left;
            break;
        case CTH:
            op_node->value = MUL;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left) return;
            op_node->left->parent = op_node;

            op_node->right = new_node(OP, DIV, nullptr, nullptr, op_node, RIGHT);
            if (op_node->right == nullptr) return;

            op_node->right->left = new_node(NUM, -1, nullptr, nullptr, op_node->right, LEFT);
            if (op_node->right->left == nullptr) return;

            op_node->right->right = new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->right = new_node(NUM, 2, nullptr, nullptr, op_node->right->right, RIGHT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left = new_node(OP, SH, nullptr, nullptr, op_node->right->right, LEFT);
            if (op_node->right->right == nullptr) return;

            op_node->right->right->left->right = new_node(NIL, NULL, nullptr, nullptr, op_node->right->right->left, RIGHT);
            if (op_node->right->right->left->right == nullptr) return;

            op_node->right->right->left->left = copy_subtree(node->left);
            if (op_node->right->right->left->left) return;
            op_node->right->right->left->left->parent = op_node->right->right->left;
            break;
        case ARCSIN:
            op_node->value = DIV;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(OP, POW, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            if (new_node(NUM, 0.5, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

            if (new_node(OP, SUB, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            if (new_node(NUM, 1, nullptr, nullptr, op_node->right->left, LEFT) == nullptr) return;

            if (new_node(OP, POW, nullptr, nullptr, op_node->right->left, RIGHT) == nullptr) return;

            if (new_node(NUM, 2, nullptr, nullptr, op_node->right->left->right, RIGHT) == nullptr) return;

            op_node->right->left->right->left = copy_subtree(node->left);
            if (op_node->right->left->right->left == nullptr) return;
            op_node->right->left->right->left->parent = op_node->right->left->right;
            break;
        case ARCCOS:
            op_node->value = MUL;

            if (new_node(NUM, -1, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            node->value = ARCSIN;
            op_node->right = differentiate(ostream, node);
            node->value = ARCCOS;
            if (op_node->right == nullptr) return;
            op_node->right->parent = op_node;
            break;
        case ARCTG:
            op_node->value = DIV;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(OP, SUB, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            if (new_node(NUM, 1, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            if (new_node(OP, MUL, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

            if (new_node(NUM, -1, nullptr, nullptr, op_node->right->right, LEFT) == nullptr) return;

            if (new_node(OP, POW, nullptr, nullptr, op_node->right->right, RIGHT) == nullptr) return;

            if (new_node(NUM, 2, nullptr, nullptr, op_node->right->right->right, RIGHT) == nullptr) return;

            op_node->right->right->left = copy_subtree(node->left);
            if (op_node->right->right->left == nullptr) return;
            op_node->right->right->left->parent = op_node->right->right;
            break;
        case ARCCTG:
            op_node->value = MUL;

            if (new_node(NUM, -1, nullptr, nullptr, op_node, LEFT) == nullptr) return;

            node->value = ARCTG;
            op_node->right = differentiate(ostream, node);
            node->value = ARCCTG;
            if (op_node->right == nullptr) return;
            op_node->right->parent = op_node;
            break;
        case ARCSH:
            op_node->value = DIV;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(OP, POW, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            if (new_node(NUM, 0.5, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

            if (new_node(OP, ADD, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            if (new_node(NUM, 1, nullptr, nullptr, op_node->right->left, LEFT) == nullptr) return;

            if (new_node(OP, POW, nullptr, nullptr, op_node->right->left, RIGHT) == nullptr) return;

            if (new_node(NUM, 2, nullptr, nullptr, op_node->right->left->right, RIGHT) == nullptr) return;

            op_node->right->left->right->left = copy_subtree(node->left);
            if (op_node->right->left->right->left == nullptr) return;
            op_node->right->left->right->left->parent = op_node->right->left->right;
            break;
        case ARCCH:
            op_node->value = DIV;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(OP, POW, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            if (new_node(NUM, 0.5, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

            if (new_node(OP, ADD, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            if (new_node(NUM, -1, nullptr, nullptr, op_node->right->left, LEFT) == nullptr) return;

            if (new_node(OP, POW, nullptr, nullptr, op_node->right->left, RIGHT) == nullptr) return;

            if (new_node(NUM, 2, nullptr, nullptr, op_node->right->left->right, RIGHT) == nullptr) return;

            op_node->right->left->right->left = copy_subtree(node->left);
            if (op_node->right->left->right->left == nullptr) return;
            op_node->right->left->right->left->parent = op_node->right->left->right;
            break;
        case ARCCTH:
            [[fallthrough]];
        case ARCTH:
            op_node->value = DIV;

            op_node->left = differentiate(ostream, node->left);
            if (op_node->left == nullptr) return;
            op_node->left->parent = op_node;

            if (new_node(OP, SUB, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

            if (new_node(NUM, 1, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

            if (new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

            if (new_node(NUM, 2, nullptr, nullptr, op_node->right->right, RIGHT) == nullptr) return;

            op_node->right->right->left = copy_subtree(node->left);
            if (op_node->right->right->left == nullptr) return;
            op_node->right->right->left->parent = op_node->right->right;
            break;
        case POW: {
            bool var_right = is_var_present_r(node->right);
            bool var_left = is_var_present_r(node->left);

            if (var_left == false && var_right == false) {
                op_node->type = NUM;
                op_node->value = 0;
                return;
            }
            else if (var_left == true && var_right == false) {
                op_node->type = OP;
                op_node->value = MUL;

                if (new_node(OP, MUL, nullptr, nullptr, op_node, RIGHT) == nullptr) return;
                op_node->left = differentiate(ostream, node->left);
                if (op_node->left == nullptr) return;
                op_node->left->parent = op_node;

                op_node->right->left = copy_subtree(node->right);
                if (op_node->right->left == nullptr) return;
                op_node->right->left->parent = op_node->right;

                if (new_node(OP, POW, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

                op_node->right->right->left = copy_subtree(node->left);
                if (op_node->right->right->left == nullptr) return;
                op_node->right->right->left->parent = op_node->right->right;
                if (new_node(OP, SUB, nullptr, nullptr, op_node->right->right, RIGHT) == nullptr) return;

                op_node->right->right->right->left = copy_subtree(node->right);
                if (op_node->right->right->right->left == nullptr) return;
                op_node->right->right->right->left->parent = op_node->right->right->right;
                if (new_node(NUM, 1, nullptr, nullptr, op_node->right->right->right, RIGHT) == nullptr) return;
            }
            else if (var_left == false && var_right == true) {
                op_node->value = MUL;

                if (new_node(OP, MUL, nullptr, nullptr, op_node, RIGHT) == nullptr) return;
                op_node->left = differentiate(ostream, node->right);
                if (op_node->left == nullptr) return;
                op_node->left->parent = op_node;

                op_node->right->left = copy_subtree(node);
                if (op_node->right->left == nullptr) return;
                op_node->right->left->parent = op_node->right;
                if (new_node(OP, LN, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;

                op_node->right->right->left = copy_subtree(node->left);
                if (op_node->right->right->left == nullptr) return;
                op_node->right->right->left->parent = op_node->right->right;
                if (new_node(NIL, NULL, nullptr, nullptr, op_node->right->right, RIGHT) == nullptr) return;
            }
            else if (var_left == true && var_right == true) {
                op_node->value = MUL;

                op_node->left = copy_subtree(node);
                if (op_node->left == nullptr) return;
                op_node->left->parent = op_node;
                if (new_node(OP, ADD, nullptr, nullptr, op_node, RIGHT) == nullptr) return;

                if (new_node(OP, MUL, nullptr, nullptr, op_node->right, RIGHT) == nullptr) return;
                if (new_node(OP, MUL, nullptr, nullptr, op_node->right, LEFT) == nullptr) return;

                op_node->right->right->left = copy_subtree(node->right);
                if (op_node->right->right->left  == nullptr) return;
                op_node->right->right->left->parent = op_node->right->right;
                if (new_node(OP, DIV, nullptr, nullptr, op_node->right->right, RIGHT) == nullptr) return;

                op_node->right->right->right->right = copy_subtree(node->left);
                if (op_node->right->right->right->right  == nullptr) return;
                op_node->right->right->right->right->parent = op_node->right->right->right;

                op_node->right->right->right->left = differentiate(ostream, node->left);
                if (op_node->right->right->right->left == nullptr) return;
                op_node->right->right->right->left->parent = op_node->right->right->right;

                op_node->right->left->left = differentiate(ostream, node->right);
                if (op_node->right->left->left == nullptr) return;
                op_node->right->left->left->parent = op_node->right->left;

                if (new_node(OP, LN, nullptr, nullptr, op_node->right->left, RIGHT) == nullptr) return;

                if (new_node(NIL, NULL, nullptr, nullptr, op_node->right->left->right, RIGHT) == nullptr) return;
                op_node->right->left->right->left = copy_subtree(node->left);
                if (op_node->right->left->right->left == nullptr) return;
                op_node->right->left->right->left->parent = op_node->right->left->right;
            }
            break;
        }
        default:
            break;
    }
    fprintf(ostream, "Differentiating \n\n");
    print_exp_to_tex(ostream, node);
    fprintf(ostream, "We get \n\n");
    print_exp_to_tex(ostream, op_node);
}

bool exp_tree_t::is_var_present_r(node_t* node) {
    if (node == nullptr) return false;

    if (node->type == VAR) {
        return true;
    }

    return (is_var_present_r(node->left) || is_var_present_r(node->right));
}

//===================================COPY================================================

node_t* exp_tree_t::copy_subtree(node_t* node) {
    if (node == nullptr) {
        return nullptr;
    }

    node_t* _new_node = new_node(node->type, node->value, nullptr, nullptr, nullptr, ROOT);
    if (_new_node == nullptr) {
        return nullptr;
    }

    _new_node->left = copy_subtree(node->left);
    if (_new_node->left != nullptr) {
        _new_node->left->parent = _new_node;
    }

    _new_node->right = copy_subtree(node->right);
    if (_new_node->right != nullptr) {
        _new_node->right->parent = _new_node;
    }
    return _new_node;
}

//===================================OPTIMIZE================================================

node_t* exp_tree_t::optimize(node_t* node) {
    if (node == nullptr) {
        return nullptr;
    }

    bool change_flag = true;
    bool basic_op_flag = false;

    while (change_flag == true) {
        change_flag = false;
        basic_op_flag = false;
        change_flag |= calculations_optimization_r(node);
        node = basic_operations_optimization_r(node, ROOT, &basic_op_flag);
        change_flag |= basic_op_flag;
    }
    return node;
}

bool exp_tree_t::calculations_optimization_r(node_t* node) {
    if (node == nullptr) {
        return false;
    }

    bool change_flag = false;

    if (node->left != nullptr) {
        change_flag |= calculations_optimization_r(node->left);
    }

    if (node->right != nullptr) {
        change_flag |= calculations_optimization_r(node->right);
    }

    if (node->type == OP && node->left != nullptr && node->right != nullptr &&
        node->left->type == NUM && node->right->type == NUM) {

        node->value = calculate_value(node->value, node->left->value, node->right->value);

        free(node->left);
        free(node->right);

        node->left = nullptr;
        node->right = nullptr;
        node->type = NUM;
        change_flag |= true;
    }
    return change_flag;
}

node_t* exp_tree_t::basic_operations_optimization_r(node_t* node, rel_t rel, bool* flag) {
    if (node == nullptr) {
        return nullptr;
    }

    if (node->left != nullptr) {
        basic_operations_optimization_r(node->left, LEFT, flag);
    }

    if (node->right != nullptr) {
        basic_operations_optimization_r(node->right, RIGHT, flag);
    }

    if (node->type == OP && node->left != nullptr && node->right != nullptr &&
       (node->left->type == NUM || node->right->type == NUM)) {
        if (node->left->type == NUM) {
            if ((int) node->left->value == 0) {
                null_val__optimization(node, LEFT, rel, flag);
            }
            else if ((int) node->left->value == 1) {
                node = one_val_optimization(node, LEFT, rel, flag);
            }
        }
        else if (node->right->type == NUM) {
            if ((int) node->right->value == 0) {
                null_val__optimization(node, RIGHT, rel, flag);
            }
            else if ((int) node->right->value == 1) {
                node = one_val_optimization(node, RIGHT, rel, flag);
            }
        }
    }
    return node;
}

void exp_tree_t::null_val__optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag) {
    if (node == nullptr) {
        return;
    }

    switch ((int) node->value) {
        case ADD:
        case SUB:
            if (parent_rel == RIGHT) {
                *flag = true;
                if (rel == LEFT) {
                    node->parent->right = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->right = node->left;
                    free(node->right);
                    free(node);
                }
            }
            break;
        case DIV:
            if (rel == RIGHT) {
                LOG(ERROR, "Division by zero err\n"); //SECTION - may check errors
                return;
            }

            [[fallthrough]];
        case MUL:
            *flag = true;
            delete_subtree_r(node->right);
            delete_subtree_r(node->left);

            node->right = nullptr;
            node->left = nullptr;

            node->type = NUM;
            node->value = 0;
            break;
        default:
            break;
    }
}

node_t* exp_tree_t::one_val_optimization(node_t* node, rel_t rel, rel_t parent_rel, bool* flag) {
    if (node == nullptr) {
        return nullptr;
    }

    switch ((int) node->value) {
        case MUL:
            *flag = true;
            if (parent_rel == LEFT) {
                if (rel == LEFT) {
                    node->parent->left = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->left = node->left;
                    free(node->right);
                    free(node);
                }
            }
            else if (parent_rel == RIGHT) {
                if (rel == LEFT) {
                    node->parent->right = node->right;
                    free(node->left);
                    free(node);
                }
                else if (rel == RIGHT) {
                    node->parent->right = node->left;
                    free(node->right);
                    free(node);
                }
            }
            else if (parent_rel == ROOT) {
                if (rel == LEFT) {
                    node_t* new_root = node->right;
                    free(node->left);
                    free(node);
                    new_root->parent = nullptr;
                    return new_root;
                }
                else if (rel == RIGHT) {
                    node_t* new_root = node->left;
                    free(node->right);
                    free(node);
                    new_root->parent = nullptr;
                    return new_root;
                }
            }
            break;
        case DIV:
            if (rel == LEFT) return node;
            *flag = true;
            if (parent_rel == LEFT) {
                node->parent->left = node->left;
            }
            else if (parent_rel == RIGHT) {
                node->parent->right = node->left;
            }
            else if (parent_rel == ROOT) {
                node_t* left = node->left;
                free(node);
                free(node->right);
                if (left == nullptr) return nullptr;
                left->parent = nullptr;
                return node->left;
            }
            free(node->right);
            free(node);
            break;
        case POW:
            if (rel == RIGHT) {
                *flag = true;
                if (parent_rel == LEFT) {
                    node->parent->left = node->left;
                }
                else if (parent_rel == RIGHT) {
                    node->parent->right = node->left;
                }

                free(node->right);
                free(node);
            }
            break;
        default:
            break;
    }
    return node;
}
