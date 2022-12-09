#include <atomic>
#include <cstddef>

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
    InsertOp(bool il, LockFreeNode *en, LockFreeNode *nn) {
        isLeft = il;
        expectedNode = en;
        newNode = nn;
    }
};

struct RotateOp {
    std::atomic<int> state{UNDECIDED};
    LockFreeNode *parent, *node, *child;
    std::atomic<Op*> pOp;
    Op *nOp, *cOp;
    bool dir, rightR;
    RotateOp(LockFreeNode *p, LockFreeNode *n, LockFreeNode *c, Op *pop, Op *nop, Op *cop, bool d, bool rr) {
        parent = p;
        node = n;
        child = c;
        pOp = pop;
        nOp = nop;
        cOp = cop;
        dir = d;
        rightR = rr;
    }
};

union Op {
    InsertOp insertOp;
    RotateOp rotateOp;
};

struct LockFreeNode {
    int key;
    std::atomic<LockFreeNode*> left{NULL}, right{NULL};
    std::atomic<Op*> op{NULL};
    int localHeight, lh, rh;
    std::atomic<bool> deleted{false}, removed{false};
};

// operation status interactions
static inline Op *UNFLAG(Op *op) {
    op = (Op*)((long)op & (~0x11L));
    return op;
}
static inline Op *FLAG(Op *op, long status) {
    op = (Op*)((long)(UNFLAG(op)) | status);
    return op;
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
    int leftRotate(LockFreeNode *parent, int dir, bool rotate);
    int rightRotate(LockFreeNode *parent, int dir, bool rotate);
    void help(LockFreeNode *parent, Op *parentOp, LockFreeNode *node, Op *nodeOp);
    void helpInsert(Op *op, LockFreeNode *dest);
    void helpMarked(LockFreeNode *parent, Op *parentOp, LockFreeNode *node);
    bool helpRotate(Op *op, LockFreeNode *parent, LockFreeNode *node, LockFreeNode *child);
};
