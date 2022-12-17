#include "CoarseAVLTree.hpp"

#include <assert.h>
#include <vector>

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
    for (int i = 2; i != 1; i = (i * 2) % 2017) {
        assert(t->insert(i));
        t->checkTree();
        assert(!t->insert(i));
        t->checkTree();
    }
    for (int i = 2; i != 1; i = (i * 2) % 2017) {
        assert(t->search(i));
        t->checkTree();
    }
    int numNodes = t->size();
    for (int i = 2; i != 1; i = (i * 2) % 2017) {
        assert(t->remove(i));
        t->checkTree();
        assert(!t->remove(i));
        assert(numNodes - 1 == t->size());
        numNodes--;
    }
    printf("Test complexSequential passed!\n");
}

int main() {
    simpleSequential();
    complexSequential();
}