all: LockFull.exe LockFree.exe Coarse.exe Performance.exe

Performance.exe: LockFullSrc.o LockFreeSrc.o Performance.o
	g++ -o Performance.exe LockFullSrc.o LockFreeSrc.o Performance.o -pthread

Performance.o: tst/Performance.cpp
	g++ -o Performance.o -Isrc -c tst/Performance.cpp -pthread

Coarse.exe: CoarseSrc.o CoarseTst.o
	g++ -o Coarse.exe CoarseSrc.o CoarseTst.o -pthread

CoarseSrc.o: src/CoarseAVLTree.cpp
	g++ -o CoarseSrc.o -c src/CoarseAVLTree.cpp

CoarseTst.o: tst/CoarseTest.cpp
	g++ -o CoarseTst.o -Isrc -c tst/CoarseTest.cpp -pthread

LockFull.exe: LockFullSrc.o LockFullTst.o
	g++ -o LockFull.exe LockFullSrc.o LockFullTst.o -pthread

LockFullSrc.o: src/LockFullAVLTree.cpp
	g++ -o LockFullSrc.o -c src/LockFullAVLTree.cpp

LockFullTst.o: tst/LockFullTest.cpp
	g++ -o LockFullTst.o -Isrc -c tst/LockFullTest.cpp -pthread

LockFree.exe: LockFreeSrc.o LockFreeTst.o
	g++ -o LockFree.exe LockFreeSrc.o LockFreeTst.o -pthread

LockFreeSrc.o: src/LockFreeAVLTree.cpp
	g++ -o LockFreeSrc.o -c src/LockFreeAVLTree.cpp

LockFreeTst.o: tst/LockFreeTest.cpp
	g++ -o LockFreeTst.o -Isrc -c tst/LockFreeTest.cpp -pthread

clean:
	rm LockFree.exe LockFull.exe Performance.exe LockFullSrc.o LockFullTst.o LockFreeSrc.o LockFreeTst.o Coarse.exe CoarseSrc.o CoarseTst.o Performance.o outputLockFull.txt
