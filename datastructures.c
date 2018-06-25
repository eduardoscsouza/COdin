#include "neuralnetwork.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <limits.h>
#include <time.h>

//Dont use fancy random for learning purposes
#define FANCY_RANDOM 0



COdin_Vector new_codin_vector(codin_size_t size)
{
	COdin_Vector new_vector;
	new_vector.size = size;
	new_vector.vector = (codin_float_t*) malloc(size * sizeof(codin_float_t));

	return new_vector;
}

COdin_Vector new_random_codin_vector(codin_size_t size, codin_float_t min, codin_float_t max)
{
	COdin_Vector new_vector = new_codin_vector(size);

	codin_float_t rand_max;
	#if FANCY_RANDOM
		rand_max = ULLONG_MAX;
	#else
		srand(time(NULL));
		rand_max = RAND_MAX;
	#endif
	
	codin_size_t i;
	unsigned long long aux_rand;
	for (i=0; i<size; i++){
		#if FANCY_RANDOM
			syscall(SYS_getrandom, &aux_rand, sizeof(aux_rand), 0);
		#else
			aux_rand = rand();
		#endif
		new_vector.vector[i] = ((aux_rand / rand_max)*(max-min)) + min;
	}

	return new_vector;
}

COdin_Vector copy_codin_vector(COdin_Vector vector)
{
	COdin_Vector new_vector = new_codin_vector(vector.size);
	memcpy(new_vector.vector, vector.vector, vector.size*sizeof(codin_float_t));
}

void delete_codin_vector(COdin_Vector vector)
{
	free(vector.vector);
	COdin_Vector.size = 0;
}

void print_codin_vector(COdin_Vector vector)
{
	printf("[");
	codin_size_t i;
	for (i=0; i<vector.size; i++) printf("%f, ", );
	printf("]\n", );
}