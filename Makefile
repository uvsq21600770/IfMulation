run: run1


## SIMUL 1
run1: simul1
	./simul1

simul1: simul1.c
	gcc -o simul1 -Wall simul1.c -lm

## SIMUL 2
run2: mm1_2.pdf
	qpdfview mm1_2.pdf

mm1_2.pdf: mm1_2.data mm1_2.gplt
	gnuplot mm1_2.gplt

mm1_2.data: simul2
	./simul2

simul2: simul2.c
	gcc -o simul2 -Wall simul2.c -lm

## SIMUL 3
run3: mm1_3.pdf
	qpdfview mm1_3.pdf

mm1_3.pdf: mm1_3.data mm1_3.gplt
	gnuplot mm1_3.gplt

mm1_3.data: simul3
	./simul3

simul3: simul3.c
	gcc -o simul3 -Wall simul3.c -lm

## clean
clean:
	rm -f simul1
	rm -f simul2
	rm -f mm1_2.data mm1_2.pdf
	rm -f simul3
	rm -f mm1_3.data mm1_3.pdf

