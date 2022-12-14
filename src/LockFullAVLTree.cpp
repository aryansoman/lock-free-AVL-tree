#include <mutex>
#include <vector>
#include <stack>
#include <iostream>

#include "LockFullAVLTree.hpp"

int lb(LockFullNode *u) {
    return (u->rbf == -1) ? -1 : 0;
}

int rb(LockFullNode *u) {
    return (u->rbf == 1) ? 1 : 0;
}

LockFullAVLTree::LockFullAVLTree() {
    root = new LockFullNode(0, false, 0, 0);
}

bool LockFullAVLTree::search(int key) {
    root->lock.lock();
    LockFullNode *cur = root;
    LockFullNode *prev;
    while (!cur->valid) {
        if (key <= cur->key) {
            if (cur->left != NULL) {
                cur->left->lock.lock();
                prev = cur;
                cur = cur->left;
            }
            else {
                cur->lock.unlock();
		return false;
            }
        }
        else {
            if (cur->right != NULL) {
                cur->right->lock.lock();
                prev = cur;
                cur = cur->right;
            }
            else {
                cur->lock.unlock();
                return false;
            }
        }
        prev->lock.unlock();
    }
    bool found = cur->key == key;
    cur->lock.unlock();
    return found;
}

bool LockFullAVLTree::insert(int key) {
    root->lock.lock();
    LockFullNode *cur = root;
    LockFullNode *prev = NULL;
    while (true) {
        // already in tree
        if (cur->valid && key == cur->key) {
            cur->lock.unlock();
            if (prev != NULL) prev->lock.unlock();
            return false;
        }
        if (key <= cur->key) {
            if (cur->left != NULL) {
                cur->left->lock.lock();
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->left;
            }
            else {
                if (prev != NULL) prev->lock.unlock();
                LockFullNode *curCopy = new LockFullNode(cur->key, cur->valid, 0, cur->rbf);
                cur->key = key;
                cur->tag--;
                cur->rbf = 0;
                cur->valid = false;
                cur->right = curCopy;
                cur->left = new LockFullNode(key, true, 0, 0);
                cur->lock.unlock();
                return true;
            }
        }
        else {
            if (cur->right != NULL) {
                cur->right->lock.lock();
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->right;
            }
            else {
                if (prev != NULL) prev->lock.unlock();
                LockFullNode *curCopy = new LockFullNode(cur->key, cur->valid, 0, cur->rbf);
                cur->tag--;
                cur->rbf = 0;
                cur->valid = false;
                cur->left = curCopy;
                cur->right = new LockFullNode(key, true, 0, 0);
                cur->lock.unlock();
                return true;
            }
        }
    }
}

bool LockFullAVLTree::remove(int key) {
    root->lock.lock();
    LockFullNode *cur = root;
    LockFullNode *prev = NULL;
    while (true) {
        if (key <= cur->key) {
            // in tree
            if (cur->left != NULL) {
                cur->left->lock.lock();
                if (cur->left->valid && cur->left->key == key) { // found
                    // cur->right should be nonNULL;
                    cur->right->lock.lock();
                    // replace cur with cur->right
                    if (cur != root) {
                        // prev should be nonNULL
                        if (prev->left == cur) {
                            prev->left = cur->right;
                        }
                        else {
                            prev->right = cur->right;
                        }
                    }
                    else {
                        root = cur->right;
                    }
                    // change tag of cur->left
                    cur->right->tag += cur->tag + 1 - rb(cur);
                    cur->left->lock.unlock();
                    cur->right->lock.unlock();
                    cur->lock.unlock();
                    if (prev != NULL) prev->lock.unlock();
                    return true;
                }
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->left;
            }
            else {
                cur->lock.unlock();
                if (prev != NULL) prev->lock.unlock();
                return false;
            }
        }
        else {
            if (cur->right != NULL) {
                cur->right->lock.lock();
                if (cur->right->valid && cur->right->key == key) { // found
                    // cur->left should be nonNULL;
                    cur->left->lock.lock();
                    // replace cur with cur->left
                    if (cur != root) {
                        // prev should be nonNULL;
                        if (prev->left == cur) {
                            prev->left = cur->left;
                        }
                        else {
                            prev->right = cur->left;
                        }
                    }
                    else {
                        root = cur->left;
                    }
                    // change tag of cur->left
                    cur->left->tag += cur->tag + 1 - lb(cur);
                    cur->left->lock.unlock();
                    cur->right->lock.unlock();
                    cur->lock.unlock();
                    if (prev != NULL) prev->lock.unlock();
                    return true;
                }
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->right;
            }
            else {
                cur->lock.unlock();
                if (prev != NULL) prev->lock.unlock();
                return false;
            }
        }
    }
}


void LockFullAVLTree::getElements(std::vector<int> &elements) {
    elements.clear();
    getElementsHelper(elements, root);
}

void LockFullAVLTree::getElementsHelper(std::vector<int> &elements, LockFullNode *root) {
    if (root == NULL) return;
    getElementsHelper(elements, root->left);
    if (root->valid) elements.push_back(root->key);
    getElementsHelper(elements, root->right);
}

void LockFullAVLTree::rebalanceAt(LockFullNode *parent, LockFullNode *child) {
    // precond: both parent and child are locked, parent->tag == 0, child->tag != 0
    // precond: other child of parent is locked.
    // precond: child of other child of parent on same side as our child is locked

    bool isLeft = parent->left == child;

    // phase 1: tag value decrease
    if (isLeft) {
        if (child->tag == -1) {
            child->tag = 0;
            parent->tag = -1 - lb(parent);
            parent->rbf++;
        }
        else { // child->tag > 0
            child->tag--;
            parent->tag = -rb(parent);
            parent->rbf--;
        }
    }
    else {
        if (child->tag == 1) {
            child->tag = 0;
            parent->tag = 1 + rb(parent);
            parent->rbf--;
        }
        else { // child->tag < 0
            child->tag++;
            parent->tag = lb(parent);
            parent->rbf++;
        }
    }

    // phase 2
    if (parent->rbf == 2) {
        LockFullNode *c = parent->left;
        LockFullNode *s = parent->right;
        int ck = c->key;
        int pk = parent->key;
        if (c->tag != 0) return;
        if (c->rbf >= 0) {
            parent->key = ck;
            parent->tag -= rb(c);
            parent->rbf = c->rbf - 1;
            parent->left = c->left;
            parent->right = c;
            c->key = pk;
            c->tag = 0;
            c->rbf = rb(c) + 1;
            c->left = c->right;
            c->right = s;
        }
        else { // c->rbf == -1
            LockFullNode *g = c->right; // non-NULL;
            if (g->tag > 0) {
                g->tag--;
                c->rbf = 0;
                parent->rbf = 1;
                parent->tag++;
            }
            else if (g->tag == 0) {
                int gk = g->key;
                int sk = s->key;
                parent->key = gk;
                parent->tag++;
                parent->rbf = 0;
                parent->right = g;
                c->rbf = -lb(g);
                c->right = g->left;
                g->key = pk;
                g->left = g->right;
                g->tag = 0;
                g->rbf = rb(g);
                g->right = s;
            }
            else { // g->tag == -1
                int gk = g->key;
                int sk = s->key;
                parent->key = gk;
                parent->rbf = g->rbf;
                parent->right = g;
                c->tag = 0;
                c->rbf = -lb(g)-1;
                c->right = g->left;
                g->key = pk;
                g->left = g->right;
                g->tag = 0;
                g->rbf = rb(g)+1;
                g->right = s;
            }
        }
    }
    else if (parent->rbf == -2) {
        LockFullNode *s = parent->left;
        LockFullNode *c = parent->right;
        int ck = c->key;
        int pk = parent->key;
        if (c->tag != 0) return;
        if (c->rbf <= 0) {
            parent->key = ck;
            parent->tag += lb(c);
            parent->rbf = c->rbf + 1;
            parent->right = c->right;
            parent->left = c;
            c->key = pk;
            c->tag = 0;
            c->rbf = lb(c) - 1;
            c->right = c->left;
            c->left = s;
        }
        else { // c->rbf == 1
            LockFullNode *g = c->left; // non-NULL;
            if (g->tag < 0) {
                g->tag++;
                c->rbf = 0;
                parent->rbf = -1;
                parent->tag--;
            }
            else if (g->tag == 0) {
                int gk = g->key;
                int sk = s->key;
                parent->key = gk;
                parent->tag--;
                parent->rbf = 0;
                parent->left = g;
                c->rbf = -rb(g);
                c->left = g->right;
                g->key = pk;
                g->right = g->left;
                g->tag = 0;
                g->rbf = lb(g);
                g->left = s;
            }
            else { // g->tag == 1
                int gk = g->key;
                int sk = s->key;
                parent->key = gk;
                parent->rbf = g->rbf;
                parent->left = g;
                c->tag = 0;
                c->rbf = rb(g) + 1;
                c->left = g->right;
                g->key = pk;
                g->right = g->left;
                g->tag = 0;
                g->rbf = -lb(g)-1;
                g->left = s;
            }
        }
    }

}

long size(LockFullNode *node) {
    if (node == NULL) return 0;
    return 1L + size(node->left) + size(node->right);
}

long unbalanceHelper(LockFullNode *node, long totalSize, long &unbalance) {
    // returns size of subtree, collects unbalance in accum
    if (node == NULL) return 0;
    long sizeHere = 1 + unbalanceHelper(node->left, totalSize, unbalance) + unbalanceHelper(node->right, totalSize, unbalance);
    unbalance += (long)abs(node->tag) * (long)(totalSize - sizeHere);
    return sizeHere;
}

long LockFullAVLTree::unbalance() {
    long unbalance = 0;
    unbalanceHelper(root, size(root), unbalance);
    return unbalance;
}

void LockFullAVLTree::rebalance() {
    LockFullNode *p, *c, *s, *g;
    std::stack<LockFullNode *> stack;
    root->tag = 0;
    stack.push(root);
    while (stack.empty() == false) {
        p = stack.top();
        stack.pop();
        if (p->left != NULL) {
            stack.push(p->left);
        }
        if (p->right != NULL) {
            stack.push(p->right);
        }
        if (p->tag == 0) {
            c = p->left;
            if (c->tag != 0) {
                s = p->right;
                g = c->right;
                p->lock.lock();
                c->lock.lock();
                s->lock.lock();
                if (g != NULL) {
                    g->lock.lock();
                }
                rebalanceAt(p, c);
                p->lock.unlock();
                c->lock.unlock();
                s->lock.unlock();
                if (g != NULL) {
                    g->lock.unlock();
                }
                return;
            }
            c = p->right;
            if (c->tag != 0) {
                s = p->left;
                g = c->left;
                p->lock.lock();
                c->lock.lock();
                s->lock.lock();
                if (g != NULL) {
                    g->lock.lock();
                }
                rebalanceAt(p, c);
                p->lock.unlock();
                c->lock.unlock();
                s->lock.unlock();
                if (g != NULL) {
                    g->lock.unlock();
                }
                return;
            }
        }
    }
    return;
}

void printBT(const std::string& prefix, const LockFullNode* node, bool isLeft) {
    if (node != NULL) {
        std::cout << prefix;
        std::cout << (isLeft ? "├──" : "└──");
        std::cout << (node->valid ? "[" : "(");
        std::cout << node->key << ", ";
        std::cout << node->tag << ", ";
        std::cout << node->rbf;
        std::cout << (node->valid ? "]" : ")") << std::endl;
        printBT(prefix + (isLeft ? "│   " : "    "), node->left, true);
        printBT(prefix + (isLeft ? "│   " : "    "), node->right, false);
    }
}

void LockFullAVLTree::printTree() {
    printBT("", root, false);
}
