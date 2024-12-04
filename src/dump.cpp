#include <errno.h>
#include <string.h>
#include <assert.h>
#include "expression_tree.h"
#include "logger.h"

const size_t MAX_NAME_LEN = 40;
const size_t COMMAND_SIZE = 100;

const char* FILENAME = "tree";

static FILE** get_dump_ostream() {
    static FILE* file = nullptr;
    return &file;
}

void exp_tree_t::set_dump_ostream(FILE* ostream) {
    *get_dump_ostream() = ostream;
}

//=========================================================================================

void exp_tree_t::print_preorder_() {
    print_preorder(root_);
}

void exp_tree_t::print_inorder_() {
    print_inorder(stdout, root_, 0);
}

//=========================================================================================

void exp_tree_t::print_preorder(node_t* node) {
    if (node == nullptr) {
        return;
    }

    printf("(");

    switch (node->type) {
        case OP: {
            print_operator(stdout, node->value);
            break;
        }
        case VAR: {
            printf("x"); //FIXME - myrr meow
            break;
        }
        case NUM: {
            printf("%f", node->value);
            break;
        }
        case NIL:
        printf("nil");
            break;
        default:
            break;
    };

    if (node->left != nullptr) {
        print_preorder(node->left);
    }
    if (node->right != nullptr) {
        print_preorder(node->right);
    }

    printf(")");
}

void exp_tree_t::print_inorder(FILE* ostream, node_t* node, int parent_precedence) {
    if (node == nullptr) {
        return;
    }

    int current_precedence = 0;

    switch (node->type) {
        case OP:
            current_precedence = get_operator_precedence((int) node->value);
            break;
        case VAR:
            [[fallthrough]];
        case NUM:
            [[fallthrough]];
        case NIL:
            current_precedence = -1;
            break;
        default:
            break;
    }

    if (node->left != nullptr && !(
       (int) node->type  == OP     && (
       (int) node->value == DIV    ||
       (int) node->value == LN     ||
       (int) node->value == LOG    ||
       (int) node->value == SIN    ||
       (int) node->value == COS    ||
       (int) node->value == TG     ||
       (int) node->value == CTG    ||
       (int) node->value == SH     ||
       (int) node->value == CH     ||
       (int) node->value == TH     ||
       (int) node->value == CTH    ||
       (int) node->value == ARCSIN ||
       (int) node->value == ARCCOS ||
       (int) node->value == ARCTG  ||
       (int) node->value == ARCCTG ||
       (int) node->value == ARCCH  ||
       (int) node->value == ARCSH  ||
       (int) node->value == ARCTH  ||
       (int) node->value == ARCCTH))) {
        if (current_precedence > parent_precedence) {
            fprintf(ostream, "(");
        }

        print_inorder(ostream, node->left, current_precedence);

        if (current_precedence > parent_precedence) {
            fprintf(ostream, ")");
        }
    }

    switch (node->type) {
        case OP: {
            if ((int) node->value == DIV) {
                fprintf(ostream, "\\frac{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}{");
                print_inorder(ostream, node->right, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == LN) {
                fprintf(ostream, "\\ln{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == LOG) {
                fprintf(ostream, "\\log{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == SIN) {
                fprintf(ostream, "\\sin{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == COS) {
                fprintf(ostream, "\\cos{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == TG) {
                fprintf(ostream, "\\tan{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == CTG) {
                fprintf(ostream, "\\cot{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == SH) {
                fprintf(ostream, "\\sinh{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == CH) {
                fprintf(ostream, "\\cosh{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == TH) {
                fprintf(ostream, "\\tanh{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == CTH) {
                fprintf(ostream, "\\coth{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == ARCSIN) {
                fprintf(ostream, "\\arcsin{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == ARCCOS) {
                fprintf(ostream, "\\arccos{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == ARCTG) {
                fprintf(ostream, "\\arctan{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else if ((int) node->value == ARCCTG) {
                fprintf(ostream, "\\arccot{");
                print_inorder(ostream, node->left, current_precedence);
                fprintf(ostream, "}");
            }
            else {
                print_operator(ostream, node->value);
            }
            break;
        }
        case VAR: {
            fprintf(ostream, "x"); //FIXME - myrr meow
            break;
        }
        case NUM: {
            fprintf(ostream, "%g", node->value);
            break;
        }
        case NIL: {
            break;
        }
        default:
            break;
    };

    if (node->right != nullptr && !(
       (int) node->type  == OP     && (
       (int) node->value == DIV    ||
       (int) node->value == LN     ||
       (int) node->value == LOG    ||
       (int) node->value == SIN    ||
       (int) node->value == COS    ||
       (int) node->value == TG     ||
       (int) node->value == CTG    ||
       (int) node->value == SH     ||
       (int) node->value == CH     ||
       (int) node->value == TH     ||
       (int) node->value == CTH    ||
       (int) node->value == ARCSIN ||
       (int) node->value == ARCCOS ||
       (int) node->value == ARCTG  ||
       (int) node->value == ARCCTG ||
       (int) node->value == ARCCH  ||
       (int) node->value == ARCSH  ||
       (int) node->value == ARCTH  ||
       (int) node->value == ARCCTH))) {
        if (node->right->type == NIL) return;
        if (current_precedence > parent_precedence) {
            fprintf(ostream, "(");
        }

        print_inorder(ostream, node->right, current_precedence);

        if (current_precedence > parent_precedence) {
            fprintf(ostream, ")");
        }
    }
}

int exp_tree_t::get_operator_precedence(int operation) {
    switch (operation) {
        case ADD:
            [[fallthrough]];
        case SUB:
            return 1;
        case MUL:
            [[fallthrough]];
        case DIV:
            return 2;
        case POW:
            return 3;
        default:
            return 0;
    }
}

void exp_tree_t::print_tree_to_tex(FILE* ostream, node_t* root) {
    print_to_tex(ostream , root);
}

void exp_tree_t::print_to_tex(FILE* ostream, node_t* node) {
    static bool first_dump = true;
    if (first_dump == true) {
        fprintf(ostream, "\\documentclass{article}\n"
                         "\\author{Alina Palonskaya}\n"
                         "\\date{November 2024}\n"
                         "\\begin{document}\n"
                         "\\maketitle\n");
    }

    print_exp_to_tex(ostream, node);

    first_dump = false;
}

void exp_tree_t::print_exp_to_tex(FILE* ostream, node_t* node) {
    assert(ostream != nullptr);
    assert(node != nullptr);

    fprintf(ostream, "$ ");
    print_inorder(ostream, node, 0);
    fprintf(ostream, " $\n\n");
}

void exp_tree_t::print_derivative_to_tex(FILE* ostream, node_t* node) {
    assert(ostream != nullptr);
    assert(node != nullptr);

    fprintf(ostream, "$ (");
    print_inorder(ostream, node, 0);
    fprintf(ostream, " )` = $");
}

//=========================================================================================

void exp_tree_t::dump(node_t* root) {
    assert(root != nullptr);

    FILE* ostream = *get_dump_ostream();
    if (ostream == nullptr) {
        LOG(ERROR, "Dump ostream is nullptr, print to stdout\n");
        ostream = stdout;
    }

    fprintf(ostream, "<pre>");
    static size_t image_cnt = 0;

    char tree_filename[MAX_NAME_LEN] = {};
    char image_filename[MAX_NAME_LEN] = {};

    snprintf(tree_filename, MAX_NAME_LEN, "data/images/%s%zu.dot", FILENAME, image_cnt);
    snprintf(image_filename, MAX_NAME_LEN, "data/images/%s%zu.png", FILENAME, image_cnt);

    FILE* tree_file = fopen(tree_filename, "wb");
    if (tree_file == nullptr) {
        LOG(ERROR, "Failed to open an output file\n");
        return;
    }

    printf_tree_dot_file(tree_file, root);

    if (fclose(tree_file) == EOF) {
        LOG(ERROR, "Failed to close a %s file\n" STRERROR(errno), tree_filename);
        return;
    }

    char command[COMMAND_SIZE] = "";

    snprintf(command, sizeof(command), "dot -Tpng %s -o %s", tree_filename, image_filename);

    if (system(command) != 0) {
        LOG(ERROR, "Failed to create an image\n");
        return;
    }

    fprintf(ostream, "\n<img src = \"../%s\" width = 50%%>\n", image_filename);
    image_cnt++;
}

void exp_tree_t::printf_tree_dot_file(FILE* tree_file, node_t* node) {
    assert(tree_file != nullptr);
    assert(node != nullptr);

    fprintf(tree_file, "digraph G {\n\t"
                       "rankdir=TB;\n\t"
                       "bgcolor=\"#DDA0DD\";\n\t"
                       "splines=true;\n\t"
                       "node [shape=box, width=1, height=0.5, style=filled, bgcolor=\"#DDA0DD\"];\n\t");

    print_nodes(tree_file, node, 1);
    print_links(tree_file, node);

    fprintf(tree_file, "}\n");
}

void exp_tree_t::print_nodes(FILE* tree_file, node_t* node, size_t rank) {
    assert(tree_file != nullptr);
    assert(node != nullptr);

    fprintf(tree_file, "node%zu [label=<<table border='0' cellspacing='0' bgcolor=", (size_t) node % 10000);

    switch (node->type) {
        case OP:
            fprintf(tree_file, "'#F8C4B7'");
            break;
        case VAR:
            fprintf(tree_file, "'#B7F8CA  '");
            break;
        case NUM:
            fprintf(tree_file, "'#ADD8E6'");
            break;
        case NIL:
            fprintf(tree_file, "'#ADA8F0'");
            break;
        default:
            break;
    };

    fprintf(tree_file,
            "> <tr><td>addr: %p</td></tr>"
            "<tr><td bgcolor='black' height='1'></td></tr><tr><td>", node);

    switch (node->type) {
        case OP: {
            print_operator(tree_file, node->value);
            break;
        }
        case VAR:
            fprintf(tree_file,  "'x'"); //FIXME - myrr meow
            break;
        case NUM:
            fprintf(tree_file, "%f", node->value);
            break;
        case NIL:
            break;
        default:
            break;
    };

    fprintf(tree_file, "</td></tr><tr><td bgcolor='black' height='1'></td></tr>"
                       "<tr><td>right = %p</td></tr>"
                       "<tr><td bgcolor='black' height='1'></td></tr>"
                       "<tr><td>left = %p</td></tr>"
                       "</table>>];\n\t"
                       "rank = %zu\n", node->right, node->left, rank);

    if (node->left != nullptr) print_nodes(tree_file, node->left, rank + 1);
    if (node->right != nullptr) print_nodes(tree_file, node->right, rank + 1);
}

void exp_tree_t::print_links(FILE* tree_file, node_t* node) {
    assert(tree_file != nullptr);
    assert(node != nullptr);

    if (node->left != nullptr) {
        fprintf(tree_file, "node%zu -> node%zu [weight=10,color=\"black\"];\n\t",
                           (size_t) node % 10000, (size_t) node->left % 10000);
        print_links(tree_file, node->left);
    }
    if (node->right != nullptr) {
        fprintf(tree_file, "node%zu -> node%zu [weight=10,color=\"black\"];\n\t",
                           (size_t) node % 10000, (size_t) node->right % 10000);
        print_links(tree_file, node->right);
    }
}

//=========================================================================================

void exp_tree_t::print_operator(FILE* ostream, double value) {
    assert(ostream != nullptr);

    switch ((int) value) {
        case ADD:
            fprintf(ostream, " + ");
            break;
        case SUB:
            fprintf(ostream, " - ");
            break;
        case MUL:
            fprintf(ostream, " * ");
            break;
        case DIV:
            fprintf(ostream, " / ");
            break;
        case POW:
            fprintf(ostream, " ^ ");
            break;
        case LOG:
            fprintf(ostream, " log ");
            break;
        case LN:
            fprintf(ostream, " ln ");
            break;
        case EXP:
            fprintf(ostream, " exp ");
            break;
        case SIN:
            fprintf(ostream, " sin ");
            break;
        case COS:
            fprintf(ostream, " cos ");
            break;
        case CTG:
            fprintf(ostream, " ctg ");
            break;
        case TG:
            fprintf(ostream, " tg ");
            break;
        case SH:
            fprintf(ostream, " sinh ");
            break;
        case CH:
            fprintf(ostream, " cosh ");
            break;
        case CTH:
            fprintf(ostream, " ctanh ");
            break;
        case TH:
            fprintf(ostream, " tanh ");
            break;
        case ARCSIN:
            fprintf(ostream, " arcsin ");
            break;
        case ARCCOS:
            fprintf(ostream, " arccos ");
            break;
        case ARCTG:
            fprintf(ostream, " arctg ");
            break;
        case ARCCTG:
            fprintf(ostream, " arcctg ");
            break;
        case ARCSH:
            fprintf(ostream, " arcsinh ");
            break;
        case ARCCH:
            fprintf(ostream, " arccosh ");
            break;
        case ARCTH:
            fprintf(ostream, " arcth ");
            break;
        case ARCCTH:
            fprintf(ostream, " arccth ");
            break;
        default:
            LOG(ERROR, "Unknown sign %f was detected\n", value);
            break;
    };
}

//======================================================================================

int exp_tree_t::def_operator(char* op) {
    if (strnstr(op, "+", MAX_OP_LEN) != nullptr) {
        return ADD;
    }
    else if (strnstr(op, "-", MAX_OP_LEN) != nullptr) {
        return SUB;
    }
    else if (strnstr(op, "/", MAX_OP_LEN) != nullptr) {
        return DIV;
    }
    else if (strnstr(op, "*", MAX_OP_LEN) != nullptr) {
        return MUL;
    }
    else if (strnstr(op, "^", MAX_OP_LEN) != nullptr) {
        return POW;
    }
    else if (strnstr(op, "log", MAX_OP_LEN) != nullptr) {
        return LOG;
    }
    else if (strnstr(op, "ln", MAX_OP_LEN) != nullptr) {
        return LN;
    }
    else if (strnstr(op, "exp", MAX_OP_LEN) != nullptr) {
        return EXP;
    }
    else if (strnstr(op, "arcsin", MAX_OP_LEN) != nullptr) {
        return ARCSIN;
    }
    else if (strnstr(op, "arccos", MAX_OP_LEN) != nullptr) {
        return ARCCOS;
    }
    else if (strnstr(op, "arctg", MAX_OP_LEN) != nullptr) {
        return ARCTG;
    }
    else if (strnstr(op, "arcctg", MAX_OP_LEN) != nullptr) {
        return ARCCTG;
    }
    else if (strnstr(op, "arcsh", MAX_OP_LEN) != nullptr) {
        return ARCSH;
    }
    else if (strnstr(op, "arcch", MAX_OP_LEN) != nullptr) {
        return ARCCH;
    }
    else if (strnstr(op, "arcth", MAX_OP_LEN) != nullptr) {
        return ARCTH;
    }
    else if (strnstr(op, "arccth", MAX_OP_LEN) != nullptr) {
        return ARCCTH;
    }
    else if (strnstr(op, "sin", MAX_OP_LEN) != nullptr) {
        return SIN;
    }
    else if (strnstr(op, "cos", MAX_OP_LEN) != nullptr) {
        return COS;
    }
    else if (strnstr(op, "ctg", MAX_OP_LEN) != nullptr) {
        return CTG;
    }
    else if (strnstr(op, "tg", MAX_OP_LEN) != nullptr) {
        return TG;
    }
    else if (strnstr(op, "sh", MAX_OP_LEN) != nullptr) {
        return SH;
    }
    else if (strnstr(op, "ch", MAX_OP_LEN) != nullptr) {
        return CH;
    }
    else if (strnstr(op, "cth", MAX_OP_LEN) != nullptr) {
        return CTH;
    }
    else if (strnstr(op, "th", MAX_OP_LEN) != nullptr) {
        return TH;
    }
    else {
        LOG(ERROR, "Unknown operation %s was detected\n", op);
        return -1;
    }
}


