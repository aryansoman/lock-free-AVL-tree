#include "LockFullAVLTree.hpp"
#include "LockFreeAVLTree.hpp"
#include "ConcurrentAVLTree.hpp"

#include <assert.h>
#include <chrono>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <random>

#define MAX_THREADS (128)
#define MAX_DATASET_SIZE (1 << 28)
#define MAX_OPS (1L << 40)

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
    long numOps;
    int numThreads;
    ConcurrentAVLTree *t;
    operation *ops;
    bool done;
};

void *runThread(void *myArgument) {
    threadArg *myArg = (threadArg *) myArgument;
    ConcurrentAVLTree *t = myArg->t;
    for (long i = myArg->tid; i < myArg->numOps; i += myArg->numThreads) {
        int key = myArg->ops[i].key;
        switch (myArg->ops[i].opType) {
            case INSERT_OP:
                t->insert(key);
                break;
            case REMOVE_OP:
                t->remove(key);
                break;
            default:
                t->search(key);
        }
    }
    myArg->done = true;
    return NULL;
}

void runRebalanceWhileModifyWorking(ConcurrentAVLTree *t, threadArg *threadArgs, int numThreads) {
    const int rebalanceGranularity = 10;
    bool allDone = false;
    while (!allDone) {
        // if not all the modify threads are done running, keep rebalancing
        for (int i = 0; i < rebalanceGranularity; i++) {
            t->rebalance();
        }
        // after "rebalanceGranularity" calls of rebalance, check if the threads are done again
        allDone = true;
        for (int i = 0; i < numThreads; i++) {
            allDone = allDone && threadArgs[i].done;
        }
    }
}

int main(int argc, char **argv) {
    //const long numOps = 1 << 28;
    const long numOps = 1 << atoi(argv[6]);
    //int percentWrite = 40;
    int percentWrite = atoi(argv[2]);
    //int datasetSize = 1 << 15;
    int datasetSize = 1 << atoi(argv[3]);
    //int numThreads = 1;
    int numThreads = atoi(argv[4]);
    //bool rebalance = true;
    bool rebalance = atoi(argv[5]);
    //bool treeType = LOCK_FULL;
    bool treeType = atoi(argv[1]);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> randElem(0, datasetSize);

    assert(numOps < MAX_OPS);
    assert(numThreads < MAX_THREADS);
    assert(0 < percentWrite && percentWrite < 100);
    assert(datasetSize < MAX_DATASET_SIZE);

    // initialize tree
    ConcurrentAVLTree *t = NULL;
    switch (treeType) {
        case LOCK_FREE:
            t = new LockFreeAVLTree();
            break;
        default:
            assert(treeType == LOCK_FULL);
            t = new LockFullAVLTree();
    }

    // populate tree with about half of dataset size to simulate steady-state behavior
    for (int i = 0; i < (datasetSize >> 1); i++) {
        t->insert(randElem(gen));
    }

    std::vector<int> elements;
    t->getElements(elements);
    printf("Initialized with %d elements\n", (int)elements.size());

    // create operations
    operation *ops = new operation[numOps];
    for (int i = 0; i < numThreads; i++) {
        ops[i].key = randElem(gen);
        ops[i].opType = (rand() % 100 < percentWrite) ?
                        (rand() % 2 == 0 ? INSERT_OP : REMOVE_OP) : // write ops are split 50-50
                        SEARCH_OP;
    }

    // populate thread arguments
    threadArg *threadArgs = new threadArg[numThreads];
    for (int i = 0; i < numThreads; i++) {
        threadArgs[i].tid = i;
        threadArgs[i].numOps = numOps;
        threadArgs[i].numThreads = numThreads;
        threadArgs[i].t = t;
        threadArgs[i].ops = ops;
        threadArgs[i].done = false;
    }

    // run threads on tree and time runtime
    auto start = std::chrono::high_resolution_clock::now();
    pthread_t *threads = new pthread_t[numThreads];
    for (int i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], NULL, runThread, (void *)&threadArgs[i]);
    }
    if (rebalance) {
        runRebalanceWhileModifyWorking(t, threadArgs, numThreads);
    }
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // output time, throughout information
    printf("%ld operations in %ld microseconds for %.3lf ops/mis\n", numOps, duration.count(), (float)numOps/duration.count());
}
