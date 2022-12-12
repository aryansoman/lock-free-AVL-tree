#include "LockFreeAVLTree.hpp"

#include <assert.h>
#include <cstdio>
#include <mutex>
#include <vector>

#define INSERT_TYPE 0
#define SEARCH_TYPE 1
#define REMOVE_TYPE 2

#define NUM_THREADS 4

struct command1 {
    LockFreeAVLTree *t;
    int type;
    int tid;
    int totalElements;
    bool shouldFind;
    bool done;
};

void *processCommand1(void *command) {
    command1 *comm = (command1 *) command;
    for (int i = comm->tid; i < comm->totalElements; i += NUM_THREADS) {
        switch (comm->type) {
            case INSERT_TYPE:
                assert(comm->t->insert(i) != comm->shouldFind);
                break;
            case SEARCH_TYPE:
                assert(comm->shouldFind == comm->t->search(i));
                break;
            default:
                assert(comm->type == REMOVE_TYPE);
                assert(comm->shouldFind == comm->t->remove(i));
        }
    }
    comm->done = true;
    return NULL;
}

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

void simpleConcurrentNoRebalance() {
    printf("Starting test simpleConcurrentNoRebalance...\n");
    LockFreeAVLTree *t = new LockFreeAVLTree;
    const int totalElements = 1 << 13;

    pthread_t threads[NUM_THREADS];
    command1 commands[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        commands[i].t = t;
        commands[i].type = INSERT_TYPE;
        commands[i].tid = i;
        commands[i].totalElements = totalElements;
        commands[i].shouldFind = false;
        commands[i].done = false;
    }

    // insert all the elements
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    processCommand1(&commands[0]);
    for (int i = 1; i < NUM_THREADS; i++) {
       pthread_join(threads[i], NULL);
    }
    printf("Inserted all %d elements\n", totalElements);

    std::vector<int> elements;
    t->getElements(elements);
    printf("Numerb of elements: %d\n", (int)elements.size());

    // search for the elements
    for (int i = 0; i < NUM_THREADS; i++) {
        commands[i].type = SEARCH_TYPE;
        commands[i].shouldFind = true;
    }
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    processCommand1(&commands[0]);
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Found all elements\n");

    // remove all the elements
    for (int i = 0; i < NUM_THREADS; i++) {
        commands[i].type = REMOVE_TYPE;
    }
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    processCommand1(&commands[0]);
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Removed all elements\n");

    // search for the elements (they should not be there)
    for (int i = 0; i < NUM_THREADS; i++) {
        commands[i].type = SEARCH_TYPE;
        commands[i].shouldFind = false;
    }
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    processCommand1(&commands[0]);
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Verified nonexistence of the elements\n");
    printf("Test simpleConcurrentNoRebalance passed!\n");
}

int main() {
    simpleSequentialNoRebalance();
    simpleConcurrentNoRebalance();
}
