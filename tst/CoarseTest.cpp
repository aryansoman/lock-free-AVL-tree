#include "CoarseAVLTree.hpp"

#include <assert.h>
#include <vector>

#define NUM_THREADS 4

#define INSERT_TYPE 0
#define SEARCH_TYPE 1
#define REMOVE_TYPE 2


struct command1 {
    CoarseAVLTree *t;
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
                assert(comm->shouldFind != comm->t->insert(i));
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


void simpleSequential() {
    printf("Starting test simpleSequential...\n");
    CoarseAVLTree *t = new CoarseAVLTree();
    for (int i = 0; i < 100; i++) {
        assert(t->insert(i));
        t->checkTree();
        assert(!t->insert(i));
        t->checkTree();
    }
    for (int i = 0; i < 100; i++) {
        assert(t->search(i));
        t->checkTree();
    }
    for (int i = 0; i < 100; i += 2) {
        assert(t->remove(i));
        t->checkTree();
        assert(!t->remove(i));
        t->checkTree();
    }
    for (int i = 0; i < 100; i++) {
        assert(t->search(i) == (i % 2 == 1));
    }
    std::vector<int> elements;
    t->getElements(elements);
    assert(elements.size() == 50);
    for (int i = 0; i < 50; i++) {
        assert(elements[i] == 2 * i + 1);
    }
    printf("Test simpleSequential passed!\n");
}


void complexSequential() {
    printf("Starting test complexSequential...\n");
    CoarseAVLTree *t = new CoarseAVLTree();
    std::vector<int> elements;
    for (int i = 2; i != 1; i = (i * 2) % 2001) {
        assert(t->insert(i));
        t->checkTree();
        assert(!t->insert(i));
        t->checkTree();
    }
    for (int i = 2; i != 1; i = (i * 2) % 2001) {
        assert(t->search(i));
        t->checkTree();
    }
    int numNodes = t->size();
    for (int i = 2; i != 1; i = (i * 2) % 2001) {
        assert(t->remove(i));
        t->checkTree();
        assert(!t->remove(i));
        assert(numNodes - 1 == t->size());
        numNodes--;
    }
    printf("Test complexSequential passed!\n");
}

void simpleConcurrent() {
    printf("Starting test simpleConcurrent...\n");
    CoarseAVLTree *t = new CoarseAVLTree;
    const int totalElements = 1 << 4;

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
    printf("Inserted all elements\n");

    // // search for the elements
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

    // // remove all the elements
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

    // // search for the elements (they should not be there)
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
    printf("Test simpleConcurrent passed!\n");
}

int main() {
    simpleSequential();
    complexSequential();
    simpleConcurrent();
}