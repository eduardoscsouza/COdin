#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H



typedef unsigned short int nn_size_t;
typedef float nn_float_t;

typedef struct Neuron
{
	nn_size_t n_dim;
	nn_float_t * weights;
	nn_float_t (*actv)(nn_float_t);
}Neuron;

typedef struct Layer
{
	nn_size_t n_neurons, in_size;
	Neuron ** neurons;
	nn_float_t (*actv)(nn_float_t);
}Layer;

typedef struct Network
{
	nn_size_t n_layers, in_size;
	Layer ** layers;
}Network;



Neuron * new_neuron(nn_size_t, nn_float_t (*)(nn_float_t));
void delete_neuron(Neuron *);
nn_float_t neuron_forward(Neuron *, nn_float_t *);
void print_neuron(Neuron *);

Layer * new_layer(nn_size_t, nn_size_t, nn_float_t (*)(nn_float_t));
void delete_layer(Layer *);
nn_float_t * layer_forward(Layer *, nn_float_t *);
void print_layer(Layer *);

Network * new_network(nn_size_t, nn_size_t *, nn_float_t (**)(nn_float_t), nn_size_t);
void delete_network(Network *);
Network * copy_network(Network *);
nn_float_t * network_forward(Network *, nn_float_t *);
void print_network(Network *);



nn_float_t relu(nn_float_t);
nn_float_t soft_relu(nn_float_t);
nn_float_t step(nn_float_t);
nn_float_t sigm(nn_float_t);
nn_float_t linear(nn_float_t);



#endif