#include <algorithm>
#include <atomic>
#include <cstddef>

#include "LockFreeAVLTree.hpp"

#define FOUND 0
#define NOT_FOUND_L 1
#define NOT_FOUND_R 2

void updateHeights(LockFreeNode *node) {
    node->lh = node->left == NULL ? 0 : node->left.load()->localHeight;
    node->rh = node->right == NULL ? 0 : node->right.load()->localHeight;
    node->localHeight = 1 + std::max(node->lh, node->rh);
}

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
            helpInsert((Op*)UNFLAG(*nodeOp), *node);
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
            newNode = new LockFreeNode(key, 1, 0, 0);
        }
        bool isLeft = (res == NOT_FOUND_L);
        LockFreeNode *oldNode;
        if (isLeft) {
            oldNode = node->left;
        }
        else {
            oldNode = node->right;
        }
        casOp = (Op *) (new InsertOp(isLeft, oldNode, newNode));
        if (res == FOUND && node->deleted) {
            casOp->insertOp.isUpdate = true;
        }
        Op* expected = nodeOp;
        if ((node->op).compare_exchange_strong(expected, FLAG(casOp, INSERT))) {
            helpInsert(casOp, node);
            return true;
        }
    }
}

bool LockFreeAVLTree::remove(int key) {
    LockFreeNode *parent, *node;
    Op *parentOp, *nodeOp;
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
                bool expected = false;
                if ((node->deleted).compare_exchange_strong(expected, true)) {
                    return true;
                }
            }
        }
    }
}

void LockFreeAVLTree::help(LockFreeNode *parent, Op *parentOp, LockFreeNode *node, Op *nodeOp) {
    switch (GETFLAG(nodeOp)) {
        case INSERT:
            helpInsert(UNFLAG(nodeOp), node);
            break;
        case ROTATE:
            helpRotate(UNFLAG(parentOp), parent, node, parentOp->rotateOp.child);
            break;
        case MARK:
            helpMarked(parent, parentOp, node);
    }
}

void LockFreeAVLTree::helpInsert(Op *op, LockFreeNode *dest) {
    if (op->insertOp.isUpdate) {
        bool expected = true;
        (dest->deleted).compare_exchange_strong(expected, false);
    }
    else {
        std::atomic<LockFreeNode*> *address = op->insertOp.isLeft ? &dest->left : &dest->right;
        LockFreeNode *expected = op->insertOp.expectedNode;
        (*address).compare_exchange_strong(expected, op->insertOp.newNode);
    }
    Op *expected = FLAG(op, INSERT);
    (dest->op).compare_exchange_strong(expected, FLAG(op, NONE));
}

void LockFreeAVLTree::helpMarked(LockFreeNode *parent, Op *parentOp, LockFreeNode *node) {
    LockFreeNode *child = (node->left == NULL) ? ((node->right == NULL) ? NULL : node->right.load()) : node->left.load();
    node->removed = true;
    Op *casOp = (Op *) (new InsertOp(node == parent->left, node, child));
    Op *expected = parent->op;
    if ((parent->op).compare_exchange_strong(expected, FLAG(casOp, INSERT))) {
        helpInsert(casOp, parent);
    }
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
        // create a newNode identical to node (except children, which will be diff)
        LockFreeNode *newNode = new LockFreeNode(op->rotateOp.node->key, op->rotateOp.node->localHeight, 
                                                    op->rotateOp.node->lh, op->rotateOp.node->rh);
        bool expBool = false;
        newNode->deleted.compare_exchange_strong(expBool, op->rotateOp.node->deleted.load());
        newNode->removed.compare_exchange_strong(expBool, op->rotateOp.node->removed.load());
        Op *expectedOp = NULL;
        newNode->op.compare_exchange_strong(expectedOp, op->rotateOp.node->op.load());
        LockFreeNode *expected = NULL;
        // swap around pointers for rotation
        if (op->rotateOp.rightR) {
            // right rotate
            newNode->left.compare_exchange_strong(expected, op->rotateOp.child->right.load());
            newNode->right.compare_exchange_strong(expected, op->rotateOp.node->right.load());
            expected = op->rotateOp.child->right;
            child->right.compare_exchange_strong(expected, newNode);
            // parent pointer swing
            expected = op->rotateOp.node;
            op->rotateOp.parent->left.compare_exchange_strong(expected, child);
        }
        else {
            // left rotate symmetrical to above case
            newNode->right.compare_exchange_strong(expected, op->rotateOp.child->left.load());
            newNode->left.compare_exchange_strong(expected, op->rotateOp.node->left.load());
            expected = op->rotateOp.child->left;
            child->left.compare_exchange_strong(expected, newNode);
            // parent pointer swing
            expected = op->rotateOp.node;
            op->rotateOp.parent->right.compare_exchange_strong(expected, child);
        }

        // TODO: adjust child and parent heights
        updateHeights(op->rotateOp.child);
        updateHeights(op->rotateOp.parent);

        // adjust operation state
        int expectedInt = GRABBED;
        op->rotateOp.state.compare_exchange_strong(expectedInt, ROTATED);
        seenState = ROTATED;
    }
    if (seenState == ROTATED) {
        // clear operation fields
        Op *expected = op->rotateOp.parent->op;
        op->rotateOp.parent->op.compare_exchange_strong(expected, FLAG(op->rotateOp.parent->op, NONE));
        expected = op->rotateOp.node->op;
        op->rotateOp.node->op.compare_exchange_strong(expected, FLAG(op->rotateOp.node->op, NONE));
        expected = op->rotateOp.child->op;
        op->rotateOp.child->op.compare_exchange_strong(expected, FLAG(op->rotateOp.child->op, NONE));
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

