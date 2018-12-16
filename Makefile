wish.o: wish.c
	gcc wish.c -o wish.o

wish: wish.o 
	gcc wish.o -o wish
