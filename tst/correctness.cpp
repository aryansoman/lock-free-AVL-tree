#include "LockFullAVLTree.cpp"

#include <assert.h>

void sequential1() {
    LockFullAVLTree t = new LockFullAVLTree();
    for (int i = 0; i < 100; i++) {
        t.insert(i);
        t.insert(i);
    }
    for (int i = 0; i < 100; i++) {
        assert(t.search(i));
    }
    for (int i = 0; i < 100; i += 2) {
        t.remove(i);
        t.remove(i);
    }
    for (int i = 0; i < 100; i++) {
        assert(t.search(i) == (i % 2 == 1));
    }
    vector<int> elements;
    t.getElements(elements);
    assert(elements.length() == 50);
    for (int i = 0; i < 50; i++) {
        assert(elements[i] == 2 * i + 1);
    }
}

int main() {
    sequential1(); 
}