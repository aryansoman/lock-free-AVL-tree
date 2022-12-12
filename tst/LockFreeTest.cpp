#include "LockFreeAVLTree.hpp"

#include <assert.h>
#include <cstdio>

#define NUM_THREADS 4

void simpleSequentialNoRebalance() {
    printf("Starting test simpleSequentialNoRebalance...\n");
    LockFreeAVLTree *t = new LockFreeAVLTree();
    for (int i = 0; i < 100; i++) {
        assert(t->insert(i));
        assert(!t->insert(i));
    }
    for (int i = 0; i < 100; i++) {
        assert(t->search(i));
    }
    for (int i = 0; i < 100; i += 2) {
        assert(t->remove(i));
        assert(!t->remove(i));
    }
    for (int i = 0; i < 100; i++) {
        assert(t->search(i) == (i % 2 == 1));
    }
    printf("Test simpleSequentialNoRebalance passed!\n");
}

int main() {
    simpleSequentialNoRebalance();
}
