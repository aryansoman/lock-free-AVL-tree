#include <mutex>
#include <vector>

// rotate op states
#define UNDECIDED 0
#define GRABBED 1
#define ROTATED 2

// operation statuses
#define NONE 0
#define MARK 1
#define ROTATE 2
#define INSERT 3

struct LockFreeNode;
union Op;

struct InsertOp {
    bool isLeft;
    bool isUpdate = false;
    LockFreeNode *expectedNode;
    LockFreeNode *newNode;
};

struct RotateOp {
    volatile int state = UNDECIDED;
    LockFreeNode *parent, *node, *child;
    Op *pOp, *nOp, *cOp;
    bool rightR;
    bool dir;
};

union Op {
    InsertOp insertOp;
    RotateOp rotateOp;
};

struct LockFreeNode {
    int key;
    LockFreeNode *left, *right;
    Op *op;
    int localHeight, lh, rh;
    bool deleted, removed;
};

// operation status interactions
static inline void UNFLAG(Op *&op) {
    op = (Op*)((long)op & (~0x11L));
}
static inline void FLAG(Op *&op, long status) {
    UNFLAG(op);
    op = (Op*)((long)op | status);
}
static inline long GETFLAG(Op *op) {
    return (long)op & 0x11L;
}
// ------------------------------

class LockFreeAVLTree {
public: 
    LockFreeAVLTree();
    void insert(int key);
    void remove(int key);
    bool search(int key);
private:
    LockFreeNode *root;
    int seek(int key, LockFreeNode **parent, Op **parentOp, LockFreeNode **node, Op **nodeOp, LockFreeNode *auxRoot);
};
