all: run run2 run3

run : sim
	./sim

sim:  TAC.c simul3.c
	gcc -Wall -pedantic -o sim simul3.c TAC.c -lm -g

run2: sim2
	./sim2

sim2: TAC.c simul4.c
	gcc -Wall -pedantic -o sim2 simul4.c TAC.c -lm -g

run3: sim3
	./sim3

sim3: TAC.c simul5.c
	gcc -Wall -pedantic -o sim3 TAC.c simul5.c -lm -g
