#include <vector>

#ifndef CONCURRENT_AVL_TREE
#define CONCURRENT_AVL_TREE

class ConcurrentAVLTree {
public:
    virtual bool insert(int key) = 0;
    virtual bool search(int key) = 0;
    virtual bool remove(int key) = 0;
    virtual void rebalance() {};   
    virtual void getElements(std::vector<int> &elements) {}; // not thread-safe
};

#endif