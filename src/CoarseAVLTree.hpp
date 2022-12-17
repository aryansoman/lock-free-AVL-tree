#include <mutex>
#include <vector>

#ifndef _COARSE_AVL_TREE
#define _COARSE_AVL_TREE_

#include "ConcurrentAVLTree.hpp"

struct CoarseNode {
    int key;
    int hd;
    CoarseNode *parent, *left, *right;
    CoarseNode(int k, int h, CoarseNode *p) {
        key = k;
        hd = h;
        parent = p;
        left = NULL;
        right = NULL;
    }
};

class CoarseAVLTree : public ConcurrentAVLTree {
public: 
    CoarseAVLTree();
    bool insert(int key);
    bool remove(int key);
    bool search(int key);
    void rebalance();
    void getElements(std::vector<int> &elements); // thread safe
    void printTree();
    void checkTree();
    long size();
private:
    CoarseNode *root;
    CoarseNode *leftRotate(CoarseNode *X, CoarseNode *Z);
    CoarseNode *rightRotate(CoarseNode *X, CoarseNode *Z);
    CoarseNode *leftRightRotate(CoarseNode *X, CoarseNode *Z);
    CoarseNode *rightLeftRotate(CoarseNode *X, CoarseNode *Z);
    void shiftNodes(CoarseNode *u, CoarseNode *v);
};

#endif