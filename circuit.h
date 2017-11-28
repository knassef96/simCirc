#define MAXPINS 32
#define MAXNETS 64
#define MAXINST 64
#define MAXSINKS 16

struct instPin {
	int instIndex;
	int pinIndex;
};

struct net {
	char * name;
	struct instPin source;
	int numSinks;
	struct instPin sinks[MAXSINKS];
};

struct instance {
	char * name;
	char * moduleName;
	int numInputs;
	int numOutputs;
};

struct circuit {
	char * name;
	int numOutputs;
	char * outputs[MAXPINS];
	int numInputs;
	char * inputs[MAXPINS];
	int numNets;
	struct net nets[MAXNETS];
	int numInstances;
	struct instance instances[MAXINST];
};

struct circuit readCircuit(char * circuitName);
void printCircuit(struct circuit circ);
void freeCircuit(struct circuit circ);
int netIndex(struct circuit circ, char * netName);