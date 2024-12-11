#include "expression_tree.h"
#include "logger.h"

// TODO - верификатор: нет циклов, все что не листья - операторы, у var, num нетс детей

err_t exp_tree_t::verify(node_t* root) {
    if (root == nullptr) {
        LOG(WARNING, "Root is nullptr, tree does not exist");
        return NO_ERR;
    }

    if (root->parent != nullptr) {
        LOG(ERROR, "Invalid root, root->parent has to be nullptr, but it is %p\n", root->parent);
        return INVALID_ROOT_ERR;
    }

    if (is_tree_acyclic_r(root, nullptr) == false) {
        LOG(ERROR, "Tree is not acyclic\n");
        return CYCLIC_LINKING_ERR;
    }

    err_t tree_op_err_status = check_op_type_invariants_r(root);
    if (tree_op_err_status != NO_ERR) {
        LOG(ERROR, "Operator invariants errpor\n");
        return tree_op_err_status;
    }
    return NO_ERR;
}

err_t exp_tree_t::check_op_type_invariants_r(node_t* node) {
    if (node == nullptr) return NO_ERR;

    err_t subtree_err = NO_ERR;

    if (node->right != nullptr) {
        subtree_err = check_op_type_invariants_r(node->right);
        if (subtree_err != NO_ERR) return subtree_err;
    }

    if (node->left != nullptr) {
        subtree_err = check_op_type_invariants_r(node->left);
        if (subtree_err != NO_ERR) return subtree_err;
    }

    if (node->type == NUM || node->type == VAR) {
        if (!(node->right == nullptr && node->left == nullptr)) {
            LOG(ERROR, "Node %p with type NUM/VAR cannot have childs\n", node);
            return NUM_INVAR_ERR;
        }
    }

    if (node->type == OP) {
        if (is_unary(node->value) && (node->right == nullptr || node->left != nullptr)) {
            LOG(ERROR, "Node %p(right child %p, left child %p) is unary op, it must have only right child",
                        node, node->right, node->left);
            return UN_OP_INVAR_ERR;
        }
        else if ((int) node->value != SUB && (int) node->value != ADD &&
                (node->right == nullptr || node->left == nullptr)) {
            LOG(ERROR, "Binary operator must have two childs\n");
            return BIN_OP_INVAR_ERR;
        }
        else if ((int) node->value == SUB && node->right == nullptr) {
            LOG(ERROR, "Operation SUB does not allow right null child\n");
            return SUB_SYNTAX_ERR;
        }
        else if ((int) node->value == ADD && node->right == nullptr) {
            LOG(ERROR, "Operation ADD does not allow right null child\n");
            return ADD_SYNTAX_ERR;
        }
    }
    return NO_ERR;
}

bool exp_tree_t::is_tree_acyclic_r(node_t* node, node_t* parent) {
    if (node == nullptr) return true;

    if (node->parent != parent) {
        return false;
    }

    bool is_acyclic_right = true;
    bool is_acyclic_left = true;

    if (node->right != nullptr) {
        is_acyclic_right = is_tree_acyclic_r(node->right, node);
    }

    if (node->left != nullptr) {
        is_acyclic_left = is_tree_acyclic_r(node->left, node);
    }
    return is_acyclic_right && is_acyclic_left;
}
