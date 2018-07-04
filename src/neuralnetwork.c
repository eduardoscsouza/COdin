#include "neuralnetwork.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <omp.h>

/*
Quasiconvexity interval for sigmoid function
*/
#define INIT_RAND_MAX 2.5
#define INIT_RAND_MIN -2.5

/*
Definitions for more comples code
Should be left as is for learning purposes
*/
#define NEURON_PARALLEL 0
#define NEURON_N_THREADS 8
#define LAYER_PARALLEL 0
#define LAYER_N_THREADS 8
#define UNIX_RANDOM 0



/*
Auxiliary functions
*/

/*
Creates a vector of random numbers within the range
[INIT_RAND_MIN, INIT_RAND_MAX], inclusive.
*/
codin_float_t * random_vector(codin_size_t size, codin_float_t min, codin_float_t max)
{
	codin_float_t * vector = (codin_float_t*) malloc(size * sizeof(codin_float_t));

	codin_float_t rand_max;
	#if UNIX_RANDOM
		rand_max = ULLONG_MAX;
	#else
		rand_max = RAND_MAX;
		srand(clock());
	#endif
	
	codin_size_t i;
	unsigned long long aux_rand;
	for (i=0; i<size; i++){
		#if UNIX_RANDOM
			syscall(SYS_getrandom, &aux_rand, sizeof(aux_rand), 0);
		#else
			aux_rand = rand();
		#endif
		vector[i] = ((aux_rand / rand_max)*(max-min)) + min;
	}

	return vector;
}

/*
Allocates a matrix whose elements have element_size bytes of length.
*/
void ** alloc_matrix(size_t height, size_t width, size_t element_size)
{
	size_t i;
	void ** matrix = (void**) malloc(height * sizeof(void*));
	for (i=0; i<height; i++)
		matrix[i] = (void*) malloc(width * element_size);

	return matrix;
}

/*
Deallocates a matrix created with alloc_matrix().
*/
void free_matrix(void ** matrix, size_t height)
{
	size_t i;
	for (i=0; i<height; i++) free(matrix[i]);
	free(matrix);
}


/*
Creates a new neuron for use inside a Layer.
*/
Neuron * new_neuron(codin_size_t n_weights, codin_float_t (*activation)(codin_float_t, codin_bool_t))
{
	Neuron * neuron = (Neuron*) malloc(sizeof(Neuron));
	neuron->activation = activation;
	neuron->n_weights = n_weights;
	neuron->weights = random_vector(n_weights, INIT_RAND_MIN, INIT_RAND_MAX);
	neuron->last_input = (codin_float_t*) malloc((n_weights-1) * sizeof(codin_float_t));
	neuron->last_gradient = (codin_float_t*) malloc(n_weights * sizeof(codin_float_t));
	neuron->gradient_propagation = (codin_float_t*) malloc(n_weights * sizeof(codin_float_t));
	
	/*
	Activation of the last forward pass through this neuron.
	*/
	neuron->last_net = neuron->last_output = 0;
	memset(neuron->last_input, 0, (n_weights-1) * sizeof(codin_float_t));
	memset(neuron->last_gradient, 0, n_weights * sizeof(codin_float_t));
	memset(neuron->gradient_propagation, 0, n_weights * sizeof(codin_float_t));

	return neuron;
}

/*
Frees memory allocated for a neuron.
*/
void delete_neuron(Neuron * neuron)
{
	free(neuron->weights);
	free(neuron->last_input);
	free(neuron->last_gradient);
	free(neuron->gradient_propagation);
	free(neuron);
}

/*
Calculates the output of a neuron based on its inputs.
*/
codin_float_t neuron_forward(Neuron * neuron, codin_float_t * input)
{
	codin_float_t net = 0.0;
	codin_float_t n_mults = neuron->n_weights-1;
	#if NEURON_PARALLEL
		#pragma omp parallel num_threads(NEURON_N_THREADS)
		{
			int id = omp_get_thread_num();
			codin_size_t block_size = neuron->n_mults/omp_get_num_threads(), lower_bound = block_size*id, upper_bound = block_size*(id+1);
			if (id == omp_get_max_threads()-1) upper_bound = neuron->n_mults;

			codin_size_t i;
			for(i=lower_bound; i<upper_bound; i++){
				#pragma omp critical(neuron_sum)
				{
					net += neuron->weights[i] * input[i];;
				}
			}
		}
	#else
		codin_size_t i;
		for (i=0; i<n_mults; i++)
			net += neuron->weights[i] * input[i];
	#endif
	/*
	Include bias into net.
	*/
	net += neuron->weights[neuron->n_weights-1];
	
	memcpy(neuron->last_input, input, (neuron->n_weights-1) * sizeof(codin_float_t));
	neuron->last_net = net;
	neuron->last_output = neuron->activation(net, 0);
	return neuron->last_output;
}

/*
Prints the weights of a neuron.
*/
void print_neuron(Neuron * neuron)
{
	codin_size_t i;
	for (i=0; i<neuron->n_weights-1; i++)
		printf("Weight[%d] = %f\n", i, neuron->weights[i]);
	printf("Beta = %f\n", neuron->weights[neuron->n_weights-1]);
}


/*
Creates a new layer of neurons.
*/
Layer * new_layer(codin_size_t n_neurons, codin_size_t input_size, codin_float_t (*activation)(codin_float_t, codin_bool_t))
{
	
	Layer * layer = (Layer*) malloc(sizeof(Layer));
	layer->n_neurons = n_neurons;
	layer->input_size = input_size;
	layer->gradient_propagation = (codin_float_t**) alloc_matrix(n_neurons, input_size+1, sizeof(codin_float_t));
	layer->last_gradient = (codin_float_t**) alloc_matrix(n_neurons, input_size+1, sizeof(codin_float_t));
	layer->last_input = (codin_float_t*) malloc(input_size * sizeof(codin_float_t));
	layer->last_net = (codin_float_t*) malloc(n_neurons * sizeof(codin_float_t));
	layer->last_output = (codin_float_t*) malloc(n_neurons * sizeof(codin_float_t));
	
	/*
	Values for activation and derivatives from the last forward
	pass through this layer.
	*/
	codin_size_t i;
	memset(layer->last_input, 0, input_size * sizeof(codin_float_t));
	memset(layer->last_net, 0, n_neurons * sizeof(codin_float_t));
	memset(layer->last_output, 0, n_neurons * sizeof(codin_float_t));
	for (i=0; i<n_neurons; i++){
		memset(layer->gradient_propagation[i], 0, (input_size+1) * sizeof(codin_float_t));
		memset(layer->last_gradient[i], 0, (input_size+1) * sizeof(codin_float_t));
	}

	layer->neurons = (Neuron**) malloc(n_neurons * sizeof(Neuron*));
	for (i=0; i<n_neurons; i++)
		layer->neurons[i] = new_neuron(input_size+1, activation);

	return layer;
}

/*
Frees memory allocated for a layer and its neurons.
*/
void delete_layer(Layer * layer)
{
	codin_size_t i;
	for (i=0; i<layer->n_neurons; i++)
		delete_neuron(layer->neurons[i]);
	free(layer->neurons);
	free_matrix((void**) layer->gradient_propagation, layer->n_neurons);
	free_matrix((void**) layer->last_gradient, layer->n_neurons);
	free(layer->last_input);
	free(layer->last_net);
	free(layer->last_output);
	free(layer);
}

/*
Calculates the output of all the neurons of a layer, and returns them
as a vector.
*/
codin_float_t * layer_forward(Layer * layer, codin_float_t * input)
{
	codin_float_t * output = (codin_float_t*) malloc(layer->n_neurons * sizeof(codin_float_t));

	codin_size_t i;
	#if LAYER_PARALLEL
		//Review this paralelism
		#pragma omp parallel for private(i) num_threads(LAYER_N_THREADS)
		{
			for(i=0; i<layer->n_neurons; i++) output[i] = neuron_forward(layer->neurons[i], input);
		}
	#else
		for(i=0; i<layer->n_neurons; i++) output[i] = neuron_forward(layer->neurons[i], input);
	#endif
	
	memcpy(layer->last_input, input, layer->input_size * sizeof(codin_float_t));
	for (i=0; i<layer->n_neurons; i++){
		layer->last_net[i] = layer->neurons[i]->last_net;
		layer->last_output[i] = layer->neurons[i]->last_output;
	}
	return output;
}

/*
Prints all the weights of all the neurons in the layer.
*/
void print_layer(Layer * layer)
{
	codin_size_t i;
	for (i=0; i<layer->n_neurons; i++){
		printf("---Neuron[%d]---\n", i);
		print_neuron(layer->neurons[i]);
	}
}


/*
Creates a new neural network with the specified number of neurons in each layer.
*/
Network * new_network(codin_size_t n_layers, codin_size_t * layers_sizes, codin_float_t (**layers_activations)(codin_float_t, codin_bool_t), codin_size_t input_size)
{
	Network * network = (Network*) malloc(sizeof(Network));
	network->n_layers = n_layers;
	network->input_size = input_size;
	network->last_input = (codin_float_t*) malloc(input_size * sizeof(codin_float_t));
	network->last_output = (codin_float_t*) malloc(layers_sizes[n_layers-1] * sizeof(codin_float_t));

	memset(network->last_input, 0, input_size * sizeof(codin_float_t));
	memset(network->last_output, 0, layers_sizes[n_layers-1] * sizeof(codin_float_t));

	codin_size_t i;
	network->layers = (Layer**) malloc(network->n_layers * sizeof(Layer*));
	network->layers[0] = new_layer(layers_sizes[0], input_size, layers_activations[0]);
	for (i=1; i<n_layers; i++)
		network->layers[i] = new_layer(layers_sizes[i], layers_sizes[i-1], layers_activations[i]);

	return network;
}

/*
Frees memory allocated for a network, its layers and its neurons.
*/
void delete_network(Network * network)
{
	codin_size_t i;
	for (i=0; i<network->n_layers; i++)
		delete_layer(network->layers[i]);
	free(network->layers);
	free(network->last_input);
	free(network->last_output);
	free(network);
}

/*
Network * copy_network(Network * cur_network)
{
	int i, j;
	codin_size_t * layers_sizes = (codin_size_t*) malloc(cur_network->n_layers * sizeof(codin_size_t));
	codin_float_t (**layers_actvs)(codin_float_t) = (codin_float_t (**)(codin_float_t)) malloc(cur_network->n_layers*sizeof(codin_float_t (*)(codin_float_t)));
	for (i=0; i<cur_network->n_layers; i++){
		layers_sizes[i] = cur_network->layers[i]->n_neurons;
		layers_actvs[i] = cur_network->layers[i]->activation;
	}
	Network * network = new_network(cur_network->n_layers, layers_sizes, layers_actvs, cur_network->input_size);

	for (i=0; i<cur_network->n_layers; i++){
		for (j=0; j<cur_network->layers[i]->n_neurons; j++){
			memcpy(
				network->layers[i]->neurons[j]->weights,
				cur_network->layers[i]->neurons[j]->weights,
				(cur_network->layers[i]->neurons[j]->n_dim+1)*sizeof(codin_float_t)
			);
		}
	}

	free(layers_sizes);
	free(layers_actvs);
	return network;
}
*/

/*
Calculates the output of a network by forwarding the input through
all the layers in order.
The output of the first layer gets passed as the input to the second layer,
the second layer's output gets passed to the third one and so on.
The output of the network is defined as the output of its last layer.
*/
codin_float_t * network_forward(Network * network, codin_float_t * in)
{
	codin_size_t i;
	codin_float_t * cur_vect = layer_forward(network->layers[0], in), * next_vect = NULL;
	for (i=1; i<network->n_layers; i++){
		next_vect = layer_forward(network->layers[i], cur_vect);
		free(cur_vect);
		cur_vect = next_vect;
	}

	return next_vect;
}

/*
Prints the weights of all the neurons of the network, organized by layer.
*/
void print_network(Network * network)
{
	codin_size_t i;
	for (i=0; i<network->n_layers; i++){
		printf("------Layer[%d]------\n", i);
		print_layer(network->layers[i]);
	}
}


/*
ReLU - Rectified Linear Unit
Zero if net is negative, identity otherwise.
*/
codin_float_t relu(codin_float_t net, codin_bool_t derivative)
{
	return (net>=0.0) ? (derivative ? 1.0 : net) : 0.0;
}

/*
Softplus - smooth equivalent of ReLU
0 for very negative net, 0.5 for net = 0, identity for very positive net.
Converges quickly to its limit functions.
*/
codin_float_t softplus(codin_float_t net, codin_bool_t derivative)
{
	return derivative ? 1.0/(1.0 + exp(-net)) : log(1.0 + exp(net));
}

/*
Step - discontinuous conditional
0 if net < 0, 1 if net >= 0.
*/
codin_float_t step(codin_float_t net, codin_bool_t derivative)
{
	return (net>=0.0 && !derivative) ? 1.0 : 0.0;
}

/*
Sigmoid - continuous conditional
0 for very negative net, 1 for very positive net.
*/
codin_float_t sigm(codin_float_t net, codin_bool_t derivative)
{
	return derivative ? (1.0/(1.0 + exp(-net)))*(1.0/(1.0 + exp(net))) : 1.0/(1.0 + exp(-net));
}

/*
Linear - identity function
linear(net) = net for every net.
*/
codin_float_t linear(codin_float_t net, codin_bool_t derivative)
{
	return derivative ? 1.0 : net;
}