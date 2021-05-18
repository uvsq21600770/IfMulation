run : sim
	./sim

sim: simul3.c
	gcc -Wall -pedantic -o sim simul3.c -lm
