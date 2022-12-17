# LockFull Performance Testing
# ./Performance.exe treeType percentWrite keysetSize numThreads rebalance numOps

n=22
#echo "LockFull performance testing with 2^$n operations" >> outputLockFull.txt
#echo "------------------------------------------------------------------------" >> outputLockFull.txt
#echo "" >> outputLockFull.txt

echo "keysetSize, percentWrite, numThreads, Ops/mis" >> outputLockFull.txt
for i in 15 17 19
do
    for j in 20 40 60
    do
        for k in 1 4 16 32 64
        do
            #echo "Lockfull: keysetSize = $i , percentWrite = $j , numThreads = $k" >> outputLockFull.txt
            echo -n "$i , $j , $k , " >> outputLockFull.txt
            ./Performance.exe 0 $j $i $k 1 $n >> outputLockFull.txt
            #echo "" >> outputLockFull.txt
        done
    done
done
