# CoarseAVLTree Performance Testing
# ./Performance.exe treeType percentWrite keysetSize numThreads rebalance numOps

n=22
#echo "CoarseAVLTree performance testing with 2^$n operations" >> outputCoarse.txt
#echo "------------------------------------------------------------------------" >> outputCoarse.txt
#echo "" >> outputCoarse.txt

echo "keysetSize, percentWrite, numThreads, Ops/mis" >> outputCoarse.txt
for i in 15 17 19
do
    for j in 20 40 60
    do
        for k in 1 4 16 32 64
        do
            #echo "Coarse: keysetSize = $i , percentWrite = $j , numThreads = $k" >> outputCoarse.txt
            echo -n "$i , $j , $k , " >> outputCoarse.txt
            ./Performance.exe 2 $j $i $k 0 $n >> outputCoarse.txt
            #echo "" >> outputCoarse.txt
        done
    done
done
