#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H



typedef unsigned short int codin_size_t;
typedef float codin_float_t;

typedef struct COdin_Vector
{
	codin_size_t size;
	codin_float_t * vector;
}COdin_Vector;



COdin_Vector new_codin_vector(codin_size_t);
COdin_Vector new_random_codin_vector(codin_size_t, codin_float_t, codin_float_t);
COdin_Vector copy_codin_vector(COdin_Vector);
void delete_codin_vector(COdin_Vector);
void print_codin_vector(COdin_Vector);



#endif