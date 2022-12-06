all: main.exe

main.exe: correctness.o LockFullAVLTree.o
	g++ -o main.exe correctness.o LockFullAVLTree.o

correctness.o: tst/correctness.cpp
	g++ -Isrc -c tst/correctness.cpp

LockFullAVLTree.o: src/LockFullAVLTree.cpp
	g++ -c src/LockFullAVLTree.cpp

clean:
	rm main.exe correctness.o LockFullAVLTree.o
