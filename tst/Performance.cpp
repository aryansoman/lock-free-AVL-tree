#include "LockFullAVLTree.hpp"
#include "LockFreeAVLTree.hpp"
#include "ConcurrentAVLTree.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <random>

#define MAX_THREADS (16)
#define MAX_DATASET_SIZE (1 << 28)
#define MAX_OPS (1 << 24)

#define INSERT_OP 0
#define SEARCH_OP 1
#define REMOVE_OP 2

#define LOCK_FULL 0
#define LOCK_FREE 1

struct operation {
    int opType;
    int key;
};

struct threadArg {
    int tid;
    int numOps;
    int treeType;
    ConcurrentAVLTree *t;
    operation *ops;
};


int main(int argc, char **argv) {
    const int numOps = 1 << 20;
    int percentWrite = 40;
    int datasetSize = 1 << 15;
    int numThreads = 8;
    bool rebalance = false;
    bool treeType = LOCK_FULL;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> randNum(0, datasetSize);

    // initialize tree
    ConcurrentAVLTree *t = NULL;
    switch (treeType) {
        case LOCK_FREE:
            t = new LockFreeAVLTree();
        default:
            assert(treeType == LOCK_FULL);
            t = new LockFullAVLTree();
    }

    // populate tree with about half of dataset size to simulate steady-state behavior
    for (int i = 0; i < (datasetSize >> 1); i++) {
        t->insert(randNum(gen));
    }

    std::vector<int> elements;
    t->getElements(elements);
    printf("Initialized with %d elements\n", (int)elements.size());

    // populate thread arguments

    // run threads on tree and time runtime

    // output time, throughout information    
}
