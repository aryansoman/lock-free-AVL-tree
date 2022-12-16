all: LockFull.exe LockFree.exe Coarse.exe Performance.exe

Performance.exe: LockFullSrc.o LockFreeSrc.o Performance.o
	g++ -o Performance.exe LockFullSrc.o LockFreeSrc.o Performance.o

Performance.o: tst/Performance.cpp
	g++ -o Performance.o -Isrc -c tst/Performance.cpp

Coarse.exe: CoarseSrc.o CoarseTst.o
	g++ -g -static -o Coarse.exe CoarseSrc.o CoarseTst.o

CoarseSrc.o: src/CoarseAVLTree.cpp
	g++ -g -static -o CoarseSrc.o -c src/CoarseAVLTree.cpp

CoarseTst.o: tst/CoarseTest.cpp
	g++ -g -static -o CoarseTst.o -Isrc -c tst/CoarseTest.cpp

LockFull.exe: LockFullSrc.o LockFullTst.o
	g++ -o LockFull.exe LockFullSrc.o LockFullTst.o

LockFullSrc.o: src/LockFullAVLTree.cpp
	g++ -o LockFullSrc.o -c src/LockFullAVLTree.cpp

LockFullTst.o: tst/LockFullTest.cpp
	g++ -o LockFullTst.o -Isrc -c tst/LockFullTest.cpp

LockFree.exe: LockFreeSrc.o LockFreeTst.o
	g++ -o LockFree.exe LockFreeSrc.o LockFreeTst.o

LockFreeSrc.o: src/LockFreeAVLTree.cpp
	g++ -o LockFreeSrc.o -c src/LockFreeAVLTree.cpp

LockFreeTst.o: tst/LockFreeTest.cpp
	g++ -o LockFreeTst.o -Isrc -c tst/LockFreeTest.cpp

clean:
	rm LockFree.exe LockFull.exe Performance.exe LockFullSrc.o LockFullTst.o LockFreeSrc.o LockFreeTst.o Coarse.exe CoarseSrc.o CoarseTst.o Performance.o
