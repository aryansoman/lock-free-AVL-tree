# LockFree Performance Testing
# ./Performance.exe treeType percentWrite keysetSize numThreads rebalance numOps

n=28
echo "LockFree performance testing with 2^$n operations" >> outputLockFree.txt
echo "------------------------------------------------------------------------" >> outputLockFree.txt
echo "" >> outputLockFree.txt

for i in 15 17 19
do
    for j in 20 40 60
    do
        for k in 1 4 16 32 64
        do
            echo "Lockfree: keysetSize = $i , percentWrite = $j , numThreads = $k" >> outputLockFree.txt
            ./Performance.exe 1 $j $i $k 1 $n >> outputLockFree.txt
            echo "" >> outputLockFree.txt
        done
    done
done
