#include <atomic>
#include <cstddef>

#include "LockFreeAVLTree.hpp"

#define FOUND 0
#define NOT_FOUND_L 1
#define NOT_FOUND_R 2

bool LockFreeAVLTree::search(int key) {
    LockFreeNode *node = root;
    LockFreeNode *next = node->right;
    Op *nodeOp = node->op;
    int nodeKey;
    bool res = false;
    while (next != NULL) {
        node = next;
        nodeOp = node->op;
        nodeKey = node->key;
        if (key < nodeKey) {
            next = node->left;
        }
        else if (key > nodeKey) {
            next = node->right;
        }
        else {
            res = true;
            break;
        }
    }
    if (res && node->deleted) {
        if (GETFLAG(node->op) == INSERT) {
            if (nodeOp->insertOp.newNode->key == key) {
                return true;
            }
            return false;
        }
    }
    return res;
}

int LockFreeAVLTree::seek(int key, LockFreeNode **parent, Op **parentOp, LockFreeNode **node, Op **nodeOp, LockFreeNode *auxRoot) {
    int res, nodeKey;
    LockFreeNode *next;
    retry:
    res = NOT_FOUND_L;
    *node = auxRoot;
    *nodeOp = (*node)->op;
    if (GETFLAG(*nodeOp) != NONE) {
        if (auxRoot == root) {
            help_insert((Op*)UNFLAG(*nodeOp), *node);
            goto retry;
        }
    }
    next = (LockFreeNode*)(*node)->right;
    while (next != NULL) {
        *parent = *node;
        *parentOp = *nodeOp;
        *node = next;
        *nodeOp = (*node)->op;
        nodeKey = (*node)->key;
        if (key < nodeKey) {
            res = NOT_FOUND_L;
            next = (*node)->left;
        }
        else if (key > nodeKey) {
            res = NOT_FOUND_R;
            next = (*node)->right;
        }
        else {
            res = FOUND;
            break;
        }
    }
    if (GETFLAG(*nodeOp) != NONE) {
        help(*parent, *parentOp, *node, *nodeOp);
        goto retry;
    }
    return res;
}

bool LockFreeAVLTree::insert(int key) {
    LockFreeNode *parent, *node;
    LockFreeNode *newNode = NULL;
    Op *parentOp, *nodeOp, *casOp;
    int res;
    while (true) {
        res = seek(key, &parent, &parentOp, &node, &nodeOp, root);
        if (res == FOUND && !node->deleted) {
            return false;
        }
        if (newNode == NULL) {
            newNode = alloc_node(key);
        }
        bool isLeft = (res == NOT_FOUND_L);
        LockFreeNode *oldNode;
        if (isLeft) {
            oldNode = node->left;
        }
        else {
            oldNode = node->right;
        }
        casOp = alloc_op();
        if (res == FOUND && node->deleted) {
            casOp->insertOp.isUpdate = true;
        }
        casOp->insertOp.isLeft = isLeft;
        casOp->insertOp.expectedNode = oldNode;
        casOp->insertOp.newNode = newNode;
        if (CAS(&(node->op), nodeOp, FLAG(casOp, INSERT)) == nodeOp) {
            help_insert(casOp, node);
            return true;
        }
    }
}

bool LockFreeAVLTree::remove(int key) {
    LockFreeNode *parent, *node;
    Op *parentOp, nodeOp;
    while (true) {
        int res = seek(key, &parent, &parentOp, &node, &nodeOp, root);
        if (res != FOUND) {
            return false;
        }
        if (node->deleted) {
            if (GETFLAG(node->op) != INSERT) {
                return false;
            }
        }
        else {
            if (GETFLAG(node->op) == NONE) {
                if (CAS(&(node->deleted), false, true) == false) {
                    return true;
                }
            }
        }
    }
}

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
