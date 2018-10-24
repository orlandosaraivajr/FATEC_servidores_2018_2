All:
	gcc malloc.c -g -o malloc
	gcc teste.c -g  -o teste
	gcc teste2.c -g  -o teste2
	gcc teste3.c -g  -o teste3
	gcc memoria.c -g  -o memoria
clean:
	rm malloc teste teste2 teste3 memoria
	rm *~

valgrind:
	valgrind ./malloc

valgrind2:
	valgrind ./malloc 15

valkyrie:
	valkyrie ./malloc

valkyrie2:
	valkyrie ./malloc 15
