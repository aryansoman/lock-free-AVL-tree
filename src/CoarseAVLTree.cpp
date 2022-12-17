#include <assert.h>
#include <mutex>
#include <vector>
#include <iostream>

#include "CoarseAVLTree.hpp"

#define TREE_MAX (1 << 29)

int checkTreeHelp(CoarseNode *node) {
    if (node == NULL) return 0;
    if (node->left != NULL) {
        assert(node->left->parent == node);
        assert(node->key > node->left->key);
    }
    if (node->right != NULL) {
        assert(node->right->parent == node);
        assert(node->key < node->right->key);
    }
    int lh = checkTreeHelp(node->left);
    int rh = checkTreeHelp(node->right);
    assert(node->hd == rh - lh);
    return 1 + std::max(lh, rh);
}

void CoarseAVLTree::checkTree() {
    assert(root->parent == NULL);
    checkTreeHelp(root);
}

CoarseAVLTree::CoarseAVLTree() {
    root = new CoarseNode(TREE_MAX, 0, NULL);
}

bool CoarseAVLTree::insert(int key) {
    CoarseNode *cur = root;
    CoarseNode *inserted;
    while (true) {
        if (key == cur->key) {
            return false;
        }
        else if (key < cur->key) {
            if (cur->left != NULL) cur = cur->left;
            else {
                inserted = new CoarseNode(key, 0, cur);
                cur->left = inserted;
                break;
            }
        }
        else {
            if (cur->right != NULL) cur = cur->right;
            else {
                inserted = new CoarseNode(key, 0, cur);
                cur->right = inserted;
                break;
            }
        }
    } 
    // rebalance
    CoarseNode *Z = inserted;
    CoarseNode *G, *N;
    for (CoarseNode *X = Z->parent; X != NULL; X = Z->parent) {
        if (Z == X->right) {
            if (X->hd > 0) {
                G = X->parent;
                if (Z->hd < 0) {
                    N = rightLeftRotate(X, Z);
                }
                else {
                    N = leftRotate(X, Z);
                }
            }
            else {
                if (X->hd < 0) {
                    X->hd = 0;
                    break;
                }
                X->hd = 1;
                Z = X;
                continue;
            }
        }
        else {
            if (X->hd < 0) {
                G = X->parent;
                if (Z->hd > 0) {
                    N = leftRightRotate(X, Z);
                }
                else {
                    N = rightRotate(X, Z);
                }
            }
            else {
                if (X->hd > 0) {
                    X->hd = 0;
                    break;
                }
                X->hd = -1;
                Z = X;
                continue;
            }
        }
        N->parent = G;
        if (G != NULL) {
            if (X == G->left) G->left = N;
            else G->right = N;
        }
        else {
            root = N;
        }
        break;
    }
    return true;
}

bool CoarseAVLTree::search(int key) {
    CoarseNode *cur = root;
    while (cur != NULL) {
        if (key == cur->key) {
            return true;
        }
        else if (key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return false;
}

bool CoarseAVLTree::remove(int key) {
    CoarseNode *cur = root;
    CoarseNode *toDelete = NULL;
    // find node to delete
    while (cur != NULL) {
        if (key == cur->key) {
            toDelete = cur;
            break;
        }
        else if (key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    if (toDelete == NULL) {
        return false;
    }
    // delete the actual node
    replacement:
    bool wasLeft = (toDelete->parent != NULL && toDelete == toDelete->parent->left);
    if (toDelete->left == NULL) shiftNodes(toDelete, toDelete->right);
    else if (toDelete->right == NULL) shiftNodes(toDelete, toDelete->left);
    else {
        CoarseNode *succ = toDelete->right;
        while (succ->left != NULL) succ = succ->left;
        toDelete->key = succ->key;
        toDelete = succ;
        goto replacement;
    }
    // rebalance
    CoarseNode *G, *Z, *N;
    int b;
    bool init = true;
    for (CoarseNode *X = toDelete->parent; X != NULL; X = G) {
        G = X->parent;
        if ((init && wasLeft) || N == X->left) {
            init = false;
            if (X->hd > 0) {
                Z = X->right;
                b = Z->hd; 
                if (b < 0) {
                    N = rightLeftRotate(X, Z);
                }
                else {
                    N = leftRotate(X, Z);
                }
            }
            else {
                if (X->hd == 0) {
                    X->hd = 1;
                    break;
                }
                N = X;
                N->hd = 0;
                continue;
            }
        }
        else {
            if (X->hd < 0) {
                Z = X->left;
                b = Z->hd; 
                if (b > 0) {
                    N = leftRightRotate(X, Z);
                }
                else {
                    N = rightRotate(X, Z);
                }
            }
            else {
                if (X->hd == 0) {
                    X->hd = -1;
                    break;
                }
                N = X;
                N->hd = 0;
                continue;
            }
        }
        N->parent = G;
        if (G != NULL) {
            if (X == G->left) G->left = N;
            else G->right = N;
        }
        else {
            root = N;
        }
        if (b == 0) break;
    }
    return true;
}

void getElementsHelper(std::vector<int> &elements, CoarseNode *node) {
    if (node == NULL) return;
    getElementsHelper(elements, node->left);
    if (node->key < TREE_MAX) elements.push_back(node->key);
    getElementsHelper(elements, node->right);
}

void CoarseAVLTree::getElements(std::vector<int> &elements) {
    elements.clear();
    getElementsHelper(elements, root);
}

void CoarseAVLTree::shiftNodes(CoarseNode *u, CoarseNode *v) {
    if (u->parent == NULL) root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v != NULL) v->parent = u->parent;
}

CoarseNode *CoarseAVLTree::leftRotate(CoarseNode *X, CoarseNode *Z) {
    CoarseNode *t23 = Z->left;
    X->right = t23;
    if (t23 != NULL) {
        t23->parent = X;
    }
    Z->left = X;
    X->parent = Z;
    if (Z->hd == 0) {
        X->hd = 1;
        Z->hd = -1;
    }
    else {
        X->hd = 0;
        Z->hd = 0;
    }
    return Z;
}

CoarseNode *CoarseAVLTree::rightRotate(CoarseNode *X, CoarseNode *Z) {
    CoarseNode *t23 = Z->right;
    X->left = t23;
    if (t23 != NULL) {
        t23->parent = X;
    }
    Z->right = X;
    X->parent = Z;
    if (Z->hd == 0) {
        X->hd = -1;
        Z->hd = 1;
    }
    else {
        X->hd = 0;
        Z->hd = 0;
    }
    return Z;
}

CoarseNode *CoarseAVLTree::rightLeftRotate(CoarseNode *X, CoarseNode *Z) {
    CoarseNode *Y = Z->left;
    CoarseNode *t3 = Y->right;
    Z->left = t3;
    if (t3 != NULL) {
        t3->parent = Z;
    }
    Y->right = Z;
    Z->parent = Y;
    CoarseNode *t2 = Y->left;
    X->right = t2;
    if (t2 != NULL) {
        t2->parent = X;
    }
    Y->left = X;
    X->parent = Y;
    if (Y->hd == 0) {
        X->hd = 0;
        Z->hd = 0;
    }
    else if (Y->hd > 0) {
        X->hd = -1;
        Z->hd = 0;
    }
    else {
        X->hd = 0;
        Z->hd = 1;
    }
    Y->hd = 0;
    return Y;
}

CoarseNode *CoarseAVLTree::leftRightRotate(CoarseNode *X, CoarseNode *Z) {
    CoarseNode *Y = Z->right;
    CoarseNode *t3 = Y->left;
    Z->right = t3;
    if (t3 != NULL) {
        t3->parent = Z;
    }
    Y->left = Z;
    Z->parent = Y;
    CoarseNode *t2 = Y->right;
    X->left = t2;
    if (t2 != NULL) {
        t2->parent = X;
    }
    Y->right = X;
    X->parent = Y;
    if (Y->hd == 0) {
        X->hd = 0;
        Z->hd = 0;
    }
    else if (Y->hd < 0) {
        X->hd = 1;
        Z->hd = 0;
    }
    else {
        X->hd = 0;
        Z->hd = -1;
    }
    Y->hd = 0;
    return Y;
}

void CoarseAVLTree::rebalance() {}

void printBT(const std::string& prefix, const CoarseNode* node, bool isLeft) {
    if (node != NULL) {
        std::cout << prefix;
        std::cout << (isLeft ? "├──" : "└──");
        std::cout << "[";
        std::cout << node->key << ", ";
        std::cout << node->hd;
        std::cout << "]" << std::endl;
        printBT(prefix + (isLeft ? "│   " : "    "), node->left, true);
        printBT(prefix + (isLeft ? "│   " : "    "), node->right, false);
    }
}

void CoarseAVLTree::printTree() {
    printBT("", root, false);
}

long sizeHelp(CoarseNode *node) {
    return node == NULL ? 0 : 1L + sizeHelp(node->left) + sizeHelp(node->right);
}

long CoarseAVLTree::size() {
    return sizeHelp(root);
}