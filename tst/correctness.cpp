#include "LockFullAVLTree.hpp"

#include <assert.h>
#include <vector>

#define NUM_THREADS 4

#define INSERT_TYPE 0
#define SEARCH_TYPE 1
#define REMOVE_TYPE 2

struct command1 {
    LockFullAVLTree *t;
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
                comm->t->insert(i);
                break;
            case SEARCH_TYPE:
                assert(comm->shouldFind == comm->t->search(i));
                break;
            default:
                assert(comm->type == REMOVE_TYPE);
                comm->t->remove(i);
        }
    }
    comm->done = true;
    return NULL;
}

void runRebalanceWhileModifyWorking(LockFullAVLTree *t, command1 *comm, int n) {
    const int rebalanceGranularity = 10;
    bool allDone = false;
    while (!allDone) {
        // if not all the modify threads are done running, keep rebalancing
        for (int i = 0; i < rebalanceGranularity; i++) {
            t->rebalance();
        }
        // after "rebalanceGranularity" calls of rebalance, check if the threads are done again
        allDone = true;
        for (int i = 0; i < n; i++) {
            allDone = allDone && comm[i].done;
        }
    }
}

void simpleConcurrentNoRebalance() {
    printf("Starting test simpleConcurrentNoRebalance...\n");
    LockFullAVLTree *t = new LockFullAVLTree;
    const int totalElements = 1 << 15;

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

void simpleSequentialNoRebalance() {
    printf("Starting test simpleSequentialNoRebalance...\n");
    LockFullAVLTree *t = new LockFullAVLTree();
    for (int i = 0; i < 100; i++) {
        t->insert(i);
        t->insert(i);
    }
    for (int i = 0; i < 100; i++) {
        assert(t->search(i));
    }
    for (int i = 0; i < 100; i += 2) {
        t->remove(i);
        t->remove(i);
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
    printf("Test simpleSequentialNoRebalance passed!\n");
}

void sequentialEnsureRebalanceReducesUnbalanceToZero() {
    printf("Starting test sequentialEnsureRebalanceReducesUnbalanceToZero...\n");
    int el = -15, m = 27, h = 30; 
    // create very unbalanced but not pathological tree by m -> h then el -> m - 1
    LockFullAVLTree *t = new LockFullAVLTree();
    for (int i = m; i <= h; i++) {
        t->insert(i);
    }
    for (int i = el; i <= m - 1; i++) {
        t->insert(i);
    }
    long unbalance = t->unbalance();
    printf("Initial unbalance is: %ld\n", unbalance);
    while (unbalance > 0) { 
        // at every rebalance call make sure the unbalance decreased 
        t->rebalance();
        int newUnbalance = t->unbalance();
        assert(newUnbalance < unbalance);
        unbalance = newUnbalance;
    }
    printf("Test sequentialEnsureRebalanceReducesUnbalanceToZero passed!\n");
}

void concurrentWithRebalance() {
    printf("Starting test concurrentWithRebalance...\n");
    LockFullAVLTree *t = new LockFullAVLTree;
    const int totalElements = 1 << 15;

    // in this case these threads are all modifiers while the current threads runs rebalance
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
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    runRebalanceWhileModifyWorking(t, commands, NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
       pthread_join(threads[i], NULL);
    }
    printf("Inserted all elements\n");

    // search for the elements
    for (int i = 0; i < NUM_THREADS; i++) {
        commands[i].type = SEARCH_TYPE;
        commands[i].shouldFind = true;
        commands[i].done = false;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    runRebalanceWhileModifyWorking(t, commands, NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Found all elements\n");

    // remove all the elements
    for (int i = 0; i < NUM_THREADS; i++) {
        commands[i].type = REMOVE_TYPE;
        commands[i].done = false;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, processCommand1, (void *) &commands[i]);
    }
    runRebalanceWhileModifyWorking(t, commands, NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
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
    // runRebalanceWhileModifyWorking(t, commands, NUM_THREADS);
    for (int i = 1; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Test concurrentWithRebalance... passed!\n");
}

int main() {
    simpleSequentialNoRebalance();
    simpleConcurrentNoRebalance(); 
    // sequentialEnsureRebalanceReducesUnbalanceToZero();
    concurrentWithRebalance();
}
