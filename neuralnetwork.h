#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H



#include "datastructures.h"

typedef struct Neuron
{
	codin_size_t n_dim;
	codin_float_t * weights;
	codin_float_t (*actv)(codin_float_t);
}Neuron;

typedef struct Layer
{
	codin_size_t n_neurons, in_size;
	Neuron ** neurons;
	codin_float_t (*actv)(codin_float_t);
}Layer;

typedef struct Network
{
	codin_size_t n_layers, in_size;
	Layer ** layers;
}Network;



Neuron * new_neuron(codin_size_t, codin_float_t (*)(codin_float_t));
void delete_neuron(Neuron *);
codin_float_t neuron_forward(Neuron *, codin_float_t *);
void print_neuron(Neuron *);

Layer * new_layer(codin_size_t, codin_size_t, codin_float_t (*)(codin_float_t));
void delete_layer(Layer *);
codin_float_t * layer_forward(Layer *, codin_float_t *);
void print_layer(Layer *);

Network * new_network(codin_size_t, codin_size_t *, codin_float_t (**)(codin_float_t), codin_size_t);
void delete_network(Network *);
Network * copy_network(Network *);
codin_float_t * network_forward(Network *, codin_float_t *);
void print_network(Network *);



codin_float_t relu(codin_float_t);
codin_float_t soft_relu(codin_float_t);
codin_float_t step(codin_float_t);
codin_float_t sigm(codin_float_t);
codin_float_t linear(codin_float_t);



#endif