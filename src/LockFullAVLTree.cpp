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
    // printf("inserting key %d\n", key);
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
		if (prev != NULL) prev->lock.unlock();
		LockFullNode *curCopy = new LockFullNode(cur->key, cur->valid, 0, cur->rbf);
                cur->key = key;
		cur->tag--;
		cur->rbf = 0;
		cur->valid = false;
		cur->right = curCopy;
		cur->left = new LockFullNode(key, true, 0, 0);
		cur->lock.unlock();
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
		if (prev != NULL) prev->lock.unlock();
		LockFullNode *curCopy = new LockFullNode(cur->key, cur->valid, 0, cur->rbf);
		cur->tag--;
		cur->rbf = 0;
		cur->valid = false;
		cur->left = curCopy;
		cur->right = new LockFullNode(key, true, 0, 0);
		cur->lock.unlock();
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
