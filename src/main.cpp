#include <cstdlib>
#include <errno.h>
#include <string.h>
#include "expression_tree.h"
#include "logger.h"

const char* del_images = "./del_images.sh";

int main() {
    FILE* logger = fopen("data/logger.txt", "w");
    if (logger == nullptr) {
        LOG(ERROR, "Failed to open a logger ostream\n");
        return 1;
    }

    LoggerSetFile(logger);
    LoggerSetLevel(INFO);

    int system_execution_status = system(del_images);
    if (system_execution_status == -1 || system_execution_status == 127) {
        LOG(ERROR, "Failed to execute bash script %s\n", del_images);
    }

    FILE* file = fopen("data/dump.html", "wb");
    if (file == nullptr) {
        LOG(ERROR, "Failed to open a dump ostream\n");
        return 1;
    }

    FILE* istream = fopen("data/input/data.txt", "r");
    if (istream == nullptr) {
        LOG(ERROR, "Failed to open an input data file\n");
        return 1;
    }

    FILE* tex = fopen("data/output/exp.tex", "w");
    if (tex == nullptr) {
        LOG(ERROR, "Failed to open a tex file\n");
        return 1;
    }
    exp_tree_t tree = {};

    tree.set_dump_ostream(file);
    tree.init(istream);

    tree.dump_tree();
    node_t* new_root = tree.differentiate_expression(tex);
    tree.dump(new_root);

//     node_t node1 = {};
//     node_t node2 = {};
//     node_t node3 = {};
//     node1.parent = nullptr;
//     node1.right = &node2;
//     node1.left = &node3;
//
//     node2.parent = &node1;
//     node2.right = nullptr;
//     node2.left = &node1;
//
//     node3.right = nullptr;
//     node3.left = nullptr;
//     node3.parent = &node1;
//     tree.verify(&node1);

    new_root = tree.optimize(new_root);
    tree.print_exp_to_tex(tex, new_root);
    tree.dump(new_root);

    tree.dtor();
    tree.delete_tree(new_root);

    fprintf(tex, "\n\\end{document}\n");
    if (fclose(tex) == EOF) {
        LOG(ERROR, "Failed to close tex file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(file) == EOF) {
        LOG(ERROR, "Failed to close html file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(istream) == EOF) {
        LOG(ERROR, "Failed to close data file\n" STRERROR(errno));
        return 1;
    }

    if (fclose(logger) == EOF) {
        fprintf(stderr, "Failed to close tex file\n" STRERROR(errno));
        return 1;
    }
    return 0;
}
