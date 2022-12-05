all: main.exe

main.exe: correctness.o LockFullAVLTree.o
	g++ -o main.exe correctness.o LockFullAVLTree.o

correctness.o: correctness.cpp
	g++ -c correctness.cpp

LockFullAVLTree.o: LockFullAVLTree.cpp
	g++ -c LockFullAVLTree.cpp

clean:
	rm main.exe correctness.o LockFullAVLTree.o
