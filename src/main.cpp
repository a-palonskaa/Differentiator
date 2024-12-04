#include "expression_tree.h"
#include "logger.h"

int main() {
    exp_tree_t tree;
    FILE* file = fopen("data/dump.html", "wb");
    FILE* istream = fopen("data/input/data.txt", "r");
    FILE* tex = fopen("data/output/exp.tex", "w");
    FILE* logger = fopen("data/logger.txt", "w");
    tree.set_dump_ostream(file);

    LoggerSetFile(logger);
    LoggerSetLevel(INFO);

    tree.init(istream);
    tree.dump(tree.root_);
    node_t* new_root = tree.differentiate_expression(tex);
    tree.calculations_optimization_r(new_root);
    tree.basic_operations_optimization_r(new_root, ROOT);
    tree.calculations_optimization_r(new_root);

//TODO - optimization till nothing can be optimized

    tree.dump(new_root);
    tree.dtor(new_root);
    tree.dtor(tree.root_);

    fclose(tex);
    fclose(file);
    fclose(istream);


//     node_t* root = tree.new_node(OP, DIV, nullptr, nullptr, nullptr, ROOT);
//     node_t* root_lc = tree.new_node(OP, ADD, nullptr, nullptr, root, LEFT);
//     tree.new_node(VAR, 'x', nullptr, nullptr, root_lc, LEFT);
//     tree.new_node(NUM, 3, nullptr, nullptr, root_lc, RIGHT);
//
//     node_t* root_rc = tree.new_node(OP, SUB, nullptr, nullptr, root, RIGHT);
//     tree.new_node(NUM, 1000, nullptr, nullptr, root_rc, LEFT);
//     tree.new_node(NUM, 7, nullptr, nullptr, root_rc, RIGHT);
//
//     tree.print_preorder(root);
//     tree.print_inorder(root);
//     tree.dump(root);
//     tree.dtor(root);
    return 0;
}
