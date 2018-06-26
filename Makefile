all:
	gcc neuralnetwork.c xor_test.c -fopenmp -lm -O3 -o neuralnetwork.out 

clean:
	rm *.o *.out

run:
	./neuralnetwork.out

test:
	gcc neuralnetwork.c xor_test.c -fopenmp -lm -Wall -Wextra -g -o neuralnetwork.out
	valgrind --leak-check=full --track-origins=yes ./neuralnetwork.out