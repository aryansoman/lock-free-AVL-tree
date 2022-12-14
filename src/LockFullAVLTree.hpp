#include <mutex>
#include <vector>
#include <stack>

#ifndef _LOCK_FULL_AVL_TREE_
#define _LOCK_FULL_AVL_TREE_

#include "ConcurrentAVLTree.hpp"

struct LockFullNode {
    int key;
    bool valid;
    int tag;
    int rbf;
    LockFullNode *left;
    LockFullNode *right;
    std::mutex lock;
    LockFullNode(int k, bool v, int t, int r) {
        key = k;
        valid = v;
        left = NULL;
        right = NULL;
        tag = t;
        rbf = r;
    }
};

class LockFullAVLTree : public ConcurrentAVLTree {
public:
    LockFullAVLTree();
    bool insert(int key);
    bool remove(int key);
    bool search(int key);
    void rebalance();
    void getElements(std::vector<int> &elements); // not thread-safe, use only for testing
    long unbalance(); // not thread-safe, use only for testing
private:
    LockFullNode *root;
    void rebalanceAt(LockFullNode *parent, LockFullNode *child);
    void getElementsHelper(std::vector<int> &elements, LockFullNode *node); // not thread-safe, use only for testing
};

#endif
