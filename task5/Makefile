all: main.o utils.o suff_tree.o
	gcc -std=c99 main.o utils.o suff_tree.o -o task5

main.o: main.c utils.h suff_tree.h 
	gcc -std=c99 -c main.c

utils.o: utils.c utils.h
	gcc -std=c99 -c utils.c

suff_tree.o: suff_tree.c suff_tree.h
	gcc -std=c99 -c suff_tree.c

clean:
	rm --force *.o task5

