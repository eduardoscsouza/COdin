#include "neuralnetwork.h"

//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <limits.h>

#define NEURON_PARALLEL 0
#define NEURON_N_THREADS 8
#define LAYER_PARALLEL 0
#define LAYER_N_THREADS 8

#define INIT_MAX 1.0
#define INIT_MIN -1.0



Neuron * new_neuron(nn_size_t n_dim, nn_float_t (*actv)(nn_float_t))
{
	Neuron * neuron = (Neuron*) malloc(sizeof(Neuron));
	neuron->actv = actv;
	neuron->n_dim = n_dim;
	
	neuron->weights = (nn_float_t*) malloc((n_dim+1) * sizeof(nn_float_t));
	unsigned long long aux_rand;
	nn_size_t i;
	for (i=0; i<n_dim+1; i++){
		syscall(SYS_getrandom, &aux_rand, sizeof(aux_rand), 0);
		neuron->weights[i] = ((aux_rand / (nn_float_t)ULLONG_MAX)*(INIT_MAX-INIT_MIN)) + INIT_MIN;
	}

	return neuron;
}

void delete_neuron(Neuron * neuron)
{
	free(neuron->weights);
	free(neuron);
}

nn_float_t neuron_forward(Neuron * neuron, nn_float_t * input)
{
	nn_float_t net = 0.0;
	#if NEURON_PARALLEL
		#pragma omp parallel num_threads(NEURON_N_THREADS)
		{
			int id = omp_get_thread_num();
			nn_size_t block_size = neuron->n_dim/omp_get_num_threads(), lower_bound = block_size*id, upper_bound = block_size*(id+1);
			if (id == omp_get_max_threads()-1) upper_bound = neuron->n_dim;

			nn_float_t aux;
			nn_size_t i;
			for(i=lower_bound; i<upper_bound; i++){
				aux = neuron->weights[i] * input[i];
				#pragma omp critical(neuron_sum)
				{
					net += aux;
				}
			}
		}
		net += neuron->weights[neuron->n_dim];
	#else
		nn_size_t i;
		for (i=0; i<neuron->n_dim; i++) net += neuron->weights[i] * input[i];
		net += neuron->weights[neuron->n_dim];
	#endif

	return neuron->actv(net);
}

void print_neuron(Neuron * neuron)
{
	int i;
	for (i=0; i<neuron->n_dim; i++) printf("Weight[%d] = %f\n", i, neuron->weights[i]);
	printf("Beta = %f\n", neuron->weights[neuron->n_dim]);
}



Layer * new_layer(nn_size_t n_neurons, nn_size_t in_size, nn_float_t (*actv)(nn_float_t))
{
	Layer * layer = (Layer*) malloc(sizeof(Layer));
	layer->n_neurons = n_neurons;
	layer->in_size = in_size;
	layer->actv = actv;

	layer->neurons = (Neuron**) malloc(n_neurons*sizeof(Neuron*));
	int i;
	for (i=0; i<layer->n_neurons; i++) layer->neurons[i] = new_neuron(in_size, actv);

	return layer;
}

void delete_layer(Layer * layer)
{
	int i;
	for (i=0; i<layer->n_neurons; i++) delete_neuron(layer->neurons[i]);
	free(layer->neurons);
	free(layer);
}

nn_float_t * layer_forward(Layer * layer, nn_float_t * input)
{
	nn_float_t * output = (nn_float_t*) malloc(layer->n_neurons*sizeof(nn_float_t));

	int i;
	#if LAYER_PARALLEL
		#pragma omp parallel for private(i) num_threads(LAYER_N_THREADS)
	#endif
	for(i=0; i<layer->n_neurons; i++) output[i] = neuron_forward(layer->neurons[i], input);

	return output;
}

void print_layer(Layer * layer)
{
	int i;
	for (i=0; i<layer->n_neurons; i++){
		printf("---Neuron[%d]---\n", i);
		print_neuron(layer->neurons[i]);
	}
}



Network * new_network(nn_size_t n_layers, nn_size_t * layers_sizes, nn_float_t (**layers_actvs)(nn_float_t), nn_size_t in_size)
{
	Network * network = (Network*) malloc(sizeof(Network));
	network->n_layers = n_layers;
	network->in_size = in_size;

	network->layers = (Layer**) malloc(network->n_layers*sizeof(Layer*));
	int i;
	network->layers[0] = new_layer(layers_sizes[0], network->in_size, layers_actvs[0]);
	for (i=1; i<network->n_layers; i++) network->layers[i] = new_layer(layers_sizes[i], network->layers[i-1]->n_neurons, layers_actvs[i]);

	return network;
}

void delete_network(Network * network)
{
	int i;
	for (i=0; i<network->n_layers; i++) delete_layer(network->layers[i]);
	free(network->layers);
	free(network);
}

Network * copy_network(Network * cur_network)
{
	int i, j;
	nn_size_t * layers_sizes = (nn_size_t*) malloc(cur_network->n_layers*sizeof(nn_size_t));
	nn_float_t (**layers_actvs)(nn_float_t) = (nn_float_t (**)(nn_float_t)) malloc(cur_network->n_layers*sizeof(nn_float_t (*)(nn_float_t)));
	for (i=0; i<cur_network->n_layers; i++){
		layers_sizes[i] = cur_network->layers[i]->n_neurons;
		layers_actvs[i] = cur_network->layers[i]->actv;
	}
	Network * network = new_network(cur_network->n_layers, layers_sizes, layers_actvs, cur_network->in_size);

	for (i=0; i<cur_network->n_layers; i++){
		for (j=0; j<cur_network->layers[i]->n_neurons; j++){
			memcpy(
				network->layers[i]->neurons[j]->weights,
				cur_network->layers[i]->neurons[j]->weights,
				(cur_network->layers[i]->neurons[j]->n_dim+1)*sizeof(nn_float_t)
			);
		}
	}

	free(layers_sizes);
	free(layers_actvs);
	return network;
}

nn_float_t * network_forward(Network * network, nn_float_t * in)
{
	int i;
	nn_float_t * cur_vect = layer_forward(network->layers[0], in), * next_vect = NULL;
	for (i=1; i<network->n_layers; i++){
		next_vect = layer_forward(network->layers[i], cur_vect);
		free(cur_vect);
		cur_vect = next_vect;
	}

	return next_vect;
}

void print_network(Network * network)
{
	int i;
	for (i=0; i<network->n_layers; i++){
		printf("------Layer[%d]------\n", i);
		print_layer(network->layers[i]);
	}
}



nn_float_t relu(nn_float_t net)
{
	if (net>=0.0) return net;
	else return 0.0;
}

nn_float_t soft_relu(nn_float_t net)
{
	return log(1.0 + exp(net));
}

nn_float_t step(nn_float_t net)
{
	if (net>=0.0) return 1.0;
	else return 0.0;
}

nn_float_t sigm(nn_float_t net)
{
	return 1.0/(1.0 + exp(-net));
}

nn_float_t linear(nn_float_t net)
{
	return net;
}