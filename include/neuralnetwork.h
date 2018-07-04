#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H



typedef unsigned int codin_size_t;
typedef float codin_float_t;
typedef unsigned char codin_bool_t;

/*
An atomic processing unit of the network.
Calculates a "net" value based on a wheighted sum over its inputs,
as well as a fixed bias.
Its output is an activation function applied to net.
*/
typedef struct Neuron
{
	codin_size_t n_weights;
	codin_float_t * weights;

	/*
	Receives a value for net, as well as a flag which determines
	whether the function itself or its derivative should be calculated.
	*/
	codin_float_t (*activation)(codin_float_t, codin_bool_t);

	codin_float_t * last_input, * last_gradient, * gradient_propagation;
	codin_float_t last_net, last_output;
}Neuron;

/*
A layer of multiple neurons.
Each neuron of a layer has its output connected to the input of
all neurons in the next layer.
*/
typedef struct Layer
{
	codin_size_t n_neurons, input_size;
	Neuron ** neurons;

	/*
	Data is replicated between Neuron and Layer. This is done
	so that they are more independent from each other, at the cost
	of less memory effiency and harder data coherency. 
	*/
	codin_float_t ** gradient_propagation, ** last_gradient;
	codin_float_t * last_input, * last_net, * last_output;
}Layer;

/*
All a neural network is is a sequence of neuron layers.
*/
typedef struct Network
{
	codin_size_t n_layers, input_size;
	Layer ** layers;

	codin_float_t * last_input, * last_output;
}Network;



Neuron * new_neuron(codin_size_t, codin_float_t (*)(codin_float_t, codin_bool_t));
void delete_neuron(Neuron *);
codin_float_t neuron_forward(Neuron *, codin_float_t *);
void print_neuron(Neuron *);

Layer * new_layer(codin_size_t, codin_size_t, codin_float_t (*)(codin_float_t, codin_bool_t));
void delete_layer(Layer *);
codin_float_t * layer_forward(Layer *, codin_float_t *);
void print_layer(Layer *);

Network * new_network(codin_size_t, codin_size_t *, codin_float_t (**)(codin_float_t, codin_bool_t), codin_size_t);
void delete_network(Network *);
Network * copy_network(Network *);
codin_float_t * network_forward(Network *, codin_float_t *);
void print_network(Network *);



codin_float_t relu(codin_float_t, codin_bool_t);
codin_float_t softplus(codin_float_t, codin_bool_t);
codin_float_t step(codin_float_t, codin_bool_t);
codin_float_t sigm(codin_float_t, codin_bool_t);
codin_float_t linear(codin_float_t, codin_bool_t);



#endif