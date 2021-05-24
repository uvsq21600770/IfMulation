run : sim
	./sim

sim: simul3.c
	gcc -Wall -pedantic -o sim simul3.c -lm -g

run2: sim2
	./sim2

sim2: simul4.c
	gcc -Wall -pedantic -o sim2 simul4.c -lm -g

run3: sim3
	./sim3

sim3: simul5.c
	gcc -Wall -pedantic -o sim3 simul5.c -lm -g
