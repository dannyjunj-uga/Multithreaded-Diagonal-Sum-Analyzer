compile:
	gcc -Wall -pedantic-errors proj4.c -g -c -pthread
	gcc -Wall -pedantic-errors main.c -g -c
	gcc -Wall -pedantic-errors main.o proj4.o -g -o proj4.out -pthread

run:
	./proj4.out in1.txt out1.txt 10 1

clean:
	rm *.out
	rm *.o

