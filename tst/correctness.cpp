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
    return NULL;
}

void concurrent1() {
    printf("Starting test concurrent1...\n");
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
    printf("Verfied nonexistence of the elements\n");
    printf("Test concurrent1 passed!\n");
}

void sequential1() {
    printf("Starting test sequential1...\n");
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
    printf("Test sequential1 passed!\n");
}

int main() {
    sequential1(); 
    concurrent1();
}
