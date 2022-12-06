#include <atomic>
#include <cstddef>

#include "LockFreeAVLTree.hpp"

void LockFreeAVLTree::help(LockFreeNode *parent, Op *parentOp, LockFreeNode *node, Op *nodeOp) {

}

bool LockFreeAVLTree::helpRotate(Op *op, LockFreeNode *parent, LockFreeNode *node, LockFreeNode *child) {
    int seenState = op->rotateOp.state;
    retry: 
    if (seenState == UNDECIDED) {
        if (GETFLAG(node->op) == NONE || GETFLAG(node->op) == ROTATE) {
            if (GETFLAG(node->op) == NONE) {
                Op *expected = node->op;
                (op->rotateOp.node->op).compare_exchange_strong(expected, FLAG(op, ROTATE));
            }
            if (GETFLAG(node->op) == ROTATE) {
                nodeGrabbed:
                if (GETFLAG(child->op) == NONE || GETFLAG(child->op) == ROTATE) {
                    if (GETFLAG(child->op) == NONE) {
                        Op *expected = child->op;
                        (op->rotateOp.child->op).compare_exchange_strong(expected, FLAG(op, ROTATE));
                    }
                    if (GETFLAG(child->op) == ROTATE) {
                        int expectedInt = UNDECIDED;
                        (op->rotateOp.state).compare_exchange_strong(expectedInt, GRABBED);
                        seenState = GRABBED;
                    }
                    else {
                        goto nodeGrabbed;
                    }
                }
                else {
                    help(node, node->op, child, child->op);
                    goto nodeGrabbed;
                }
            }
            else {
                goto retry;
            }
        }
        else {
            help(parent, parent->op, node, node->op);
            goto retry;
        }
    }
    if (seenState == GRABBED) {
        if (op->rotateOp.rightR) {
            //TODO: // right rotate
        }
        else {
            //TODO: // left rotate
        }

        // parent pointer swing
        LockFreeNode *expected = op->rotateOp.node;
        if (op->rotateOp.rightR) {
            op->rotateOp.parent->left.compare_exchange_strong(expected, child);
        }
        else {
            op->rotateOp.parent->right.compare_exchange_strong(expected, child);
        }

        // adjust child and parent heights
        int expectedInt = GRABBED;
        op->rotateOp.state.compare_exchange_strong(expectedInt, ROTATED);
        seenState = ROTATED;
    }
    if (seenState == ROTATED) {
        //TODO: // clear parent, node, child operation
    }
    return seenState == ROTATED;
}

int LockFreeAVLTree::leftRotate(LockFreeNode *parent, int dir, bool rotate) {
    if (parent->removed) {
        return 0;
    }
    LockFreeNode *node = (dir == 1) ? parent->right : parent->left;
    if (node == NULL || node->right == NULL) {
        return 0;
    }
    if ((*(node->right)).lh - (*(node->right)).rh > 0 && !rotate) {
        return 3; // double rotation
    }
    if (GETFLAG(parent->op) == NONE) {
        Op *rotateOp = (Op *) (new RotateOp(parent, node, node->right, parent->op, node->op, (*(node->right)).op, dir, false));
        Op *expected = rotateOp->rotateOp.pOp;
        if ((parent->op).compare_exchange_strong(expected, FLAG(rotateOp, ROTATE))) {
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
    if ((*(node->left)).rh - (*(node->left)).lh > 0 && !rotate) {
        return 3; // double rotation
    }
    if (GETFLAG(parent->op) == NONE) {
        Op *rotateOp = (Op *) (new RotateOp(parent, node, node->left, parent->op, node->op, (*(node->left)).op, dir, true));
        Op *expected = rotateOp->rotateOp.pOp;
        if ((parent->op).compare_exchange_strong(expected, FLAG(rotateOp, ROTATE))) {
            helpRotate(rotateOp, parent, node, node->left);
        }
    }
    return 1;
}