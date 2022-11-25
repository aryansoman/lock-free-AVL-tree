#include "LockFullAVLTree.hpp"

#include <assert.h>
#include <vector>

void sequential1() {
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
    LockFullAVLTree *t = new LockFullAVLTree();
    sequential1(); 
}