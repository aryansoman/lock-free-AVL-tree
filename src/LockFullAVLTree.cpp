#include <mutex>
#include <vector>

#include "LockFullAVLTree.hpp"

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

void LockFullAVLTree::insert(int key) {
    root->lock.lock();
    LockFullNode *cur = root;
    LockFullNode *prev = NULL;
    while (true) {
        // already in tree
        if (cur->valid && key == cur->key) {
            cur->lock.unlock();
            if (prev != NULL) prev->lock.unlock();
            return;
        }
        if (key <= cur->key) {
            if (cur->left != NULL) {
                cur->left->lock.lock();
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->left;
            }
            else {
                LockFullNode *p = new LockFullNode(key, false, cur->tag - 1, 0);
                if (prev != NULL) {
                    if (cur == prev->right) {
                        prev->right = p;
                    }
                    else {
                        prev->left = p; 
                    }
                }
                else {
                    root = p;
                }
                p->left = new LockFullNode(key, true, 0, 0);
                p->right = cur;
                cur->tag = 0;
                cur->lock.unlock();
                if (prev != NULL) prev->lock.unlock();
                return;
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
                LockFullNode *p = new LockFullNode(key-1, false, cur->tag - 1, 0);
                if (prev != NULL) {
                    if (cur == prev->right) {
                        prev->right = p;
                    }
                    else {
                        prev->left = p; 
                    }
                }
                else {
                    root = p;
                }
                p->right = new LockFullNode(key, true, 0, 0);
                p->left = cur;
                cur->tag = 0;
                cur->lock.unlock();
                if (prev != NULL) prev->lock.unlock();
                return;
            }
        }
    }
}

void LockFullAVLTree::remove(int key) {
    root->lock.lock();
    LockFullNode *cur = root;
    LockFullNode *prev = NULL;
    while (true) {
        // in tree
        if (key <= cur->key) {
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
                    cur->right->tag += cur->tag + 1 + (cur->rbf == 1) ? 1 : 0;
                    cur->left->lock.unlock();
                    cur->right->lock.unlock();
                    cur->lock.unlock();
                    if (prev != NULL) prev->lock.unlock();
                    return;
                }
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->left;
            }
            else { 
                cur->lock.unlock();
                if (prev != NULL) prev->lock.unlock();
                return;
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
                    cur->left->tag += cur->tag + 1 + (cur->rbf == -1) ? 1 : 0;
                    cur->left->lock.unlock();
                    cur->right->lock.unlock();
                    cur->lock.unlock();
                    if (prev != NULL) prev->lock.unlock();
                    return;
                }
                if (prev != NULL) prev->lock.unlock();
                prev = cur;
                cur = cur->right;
            }
            else {
                cur->lock.unlock();
                if (prev != NULL) prev->lock.unlock();
                return;
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
