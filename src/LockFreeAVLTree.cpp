#include <atomic>

#include "LockFreeAVLTree.hpp"

void helpRotate(Op *op, LockFreeNode *parent, LockFreeNode *node, LockFreeNode *child) {
    int seenState = op->rotateOp.state;
    while (true) {
        if (seenState == UNDECIDED) {
            if (GETFLAG(node->op) == NONE || GETFLAG(node->op) == ROTATE) {
                
            }
        }
    }
}

int LockFreeAVLTree::leftRotate(LockFreeNode *parent, int dir, bool rotate) {
    if (parent->removed) {
        return 0;
    }
    LockFreeNode *node = (dir == 1) ? parent->right : parent->left;
    if (node == NULL || node->right == NULL) {
        return 0;
    }
    if (node->right->lh - node->right->rh > 0 && !rotate) {
        return 3; // double rotation
    }
    if (GETFLAG(parent->op) == NONE) {
        Op *rotateOp = RotateOp(parent, node, node->right, parent->op, node->op, node->right->op, dir, false);
        if ((&(parent->op)).compare_exchange_strong(&(rotateOp->rotateOp.pOp), FLAG(rotateOp, ROTATE))) {
            helpRotate(rotateOp, parent, node, node->right);
        }
    }
    return 1;
}

int LockFreeAVLTree::rightRotate(LockFreeNode *parent, int dir, bool rotate) {
    if (parent->removed) {
        return 0;
    }
    LockFreeNode *node = (dir == 1) ? parent->left : parent->right;
    if (node == NULL || node->left == NULL) {
        return 0;
    }
    if (node->left->rh - node->left->lh > 0 && !rotate) {
        return 3; // double rotation
    }
    if (GETFLAG(parent->op) == NONE) {
        Op *rotateOp = RotateOp(parent, node, node->left, parent->op, node->op, node->left->op, dir, true);
        if ((&(parent->op)).compare_exchange_strong(&(rotateOp->rotateOp.pOp), FLAG(rotateOp, ROTATE))) {
            helpRotate(rotateOp, parent, node, node->left);
        }
    }
    return 1;
}