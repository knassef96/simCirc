#include "circuit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

//Evaluates each of the 5 basic circuits
int AND(int x, ...);
int OR(int x, ...);

int INV(int i1){return !i1;}
int GND(){return 0;}
int VDD(){return 1;}


//Returns true if string is one of the five basic circuits
int isBasicCircuit(char *str){
	return (
		strcmp(str, "AND")==0 ||
		strcmp(str, "OR")==0  ||
		strcmp(str, "INV")==0 ||
		strcmp(str, "GND")==0 ||
		strcmp(str, "VDD")==0 );
}

//	matches the instance name to a basic logic gate and returns the corresponding output
//	inputs can be set to nulll if they aren't being used to compute anything 
int doBasic(char *str, int size, int *inputs) {
	if(strcmp(str, "AND") == 0) {
		int andOp = 1;

		for(int y = 0; y < size; y++){
			andOp = andOp && inputs[y];
		}
	return andOp;
		
	}
	else if(strcmp(str, "OR") == 0) {
		int orOp = inputs[0];
		for(int b = 1; b < size; b++){
			orOp = orOp || inputs[b];
		}
	return orOp;
		
	}
	else if	(strcmp(str, "INV") == 0) {
		return INV(inputs[0]);
	}
	else if	(strcmp(str, "GND") == 0) {
		return GND();
	}
	else if	(strcmp(str, "VDD") == 0) {
		return VDD();
	}
	else
		return 0;
}

//this will get the source value for the input of a gate instance, i 
// current is the current net to get the values from
//	inputVals are the user inputted values 
//	outputVals are the current outputs of all the gate instances
char getSource(struct net current, int i, int *inputVals, int *outVals) {
	if (current.source.instIndex == -1) 
		return inputVals[current.source.pinIndex];
	else 
		return outVals[i];
}

int *evaluate(struct circuit mainCirc, int inputArray[]) {
	int *outVals;
	outVals = malloc(cnt* sizeof *outVals); // hold the output of the gate instances
	int w = 0;
	for(;w<100;w++) {
		outVals[w]=0;
	}

	//main logic that iterates through the instances and finds the correct inputs and writes to an output array
	//output array is indexed by the instance number i.e. the outout of instance 0 is in outVals[0]
	int i = 0;
	for (; i < mainCirc.numInstances; i++) {
		int inputs[100];	//inputs for the instance gate;
		int numGateInputs = 0;
		int j = 0;	
		for (; j < mainCirc.numNets; j++) {		//loop through the nets and find the input values for the current instance
				struct net current = mainCirc.nets[j];		//current net
			int k = 0;			
			for (; k < current.numSinks; k++) {	//iterate through sinks to find the right one for the gate
				if (current.sinks[k].instIndex == i) {		//if there is a match then write to gate inputs;
					inputs[current.sinks[k].pinIndex] = getSource(current, current.source.instIndex, inputArray, outVals);
					numGateInputs++;
				}	
			}
		}
		if (isBasicCircuit(mainCirc.instances[i].moduleName)) {
			outVals[i] = doBasic(mainCirc.instances[i].moduleName, numGateInputs, inputs);  //Going to have to get changed if using array of input method
			//printf("(%d,%d)\n",i,outVals[i]);
		}
		else {
			struct circuit subCirc = readCircuit(mainCirc.instances[i].moduleName);
			int *subOuts = evaluate(subCirc, inputs);
			outVals[i] = subOuts[subCirc.numInstances - 1];
		}
	}
	return outVals;
}

int main(int argc, char *argv[]){
	//Check to make sure proper number of args
	if(argc != 3)
	{
		printf("Input Error: wrong number of args\n");
		exit(1);
	}
	//Declare circuits struct and input string
	struct circuit mainCirc;
	char *inputVals;

	//Read in information
	mainCirc = readCircuit(argv[1]);
	inputVals = argv[2];
	int a = 0;
	int inputArray[32];
	for(; a<strlen(argv[2]); a++){
		if(argv[2][a] == '1'){
			inputArray[a] = 1;
			}
		else if(argv[2][a] == '0'){
			inputArray[a] = 0;
			}
		else{ printf("You may only input 1 or 0\n"); exit(1);}
	}
	
	//Print the circuit name and each of the inputs
	printf("Simulated circuit %s\n", mainCirc.name);
	printf("Inputs: ");
	int i = 0;
	for(; i<mainCirc.numInputs; i++) {
		//If the last element, replace the space with a newline char
		if(i==mainCirc.numInputs-1)
			printf("%s=%c\n", mainCirc.inputs[i], inputVals[i]);
		else
			printf("%s=%c ", mainCirc.inputs[i], inputVals[i]);
	}

	//given the input and the circuit structe find the outputs of all the instances in the circuit
	//outVals is indexed by instance number
	int *outVals = evaluate(mainCirc, inputArray);

	printf("Outputs: ");
	i = 0;
	for(; i<mainCirc.numOutputs; i++){
		int b = 0;
		for (; b < mainCirc.numNets; b++) {
			if (strcmp(mainCirc.outputs[i], mainCirc.nets[b].name) == 0) {
				//If the last element, replace the space with a newline char
				if(i==mainCirc.numOutputs - 1)
					printf("%s=%d\n", mainCirc.outputs[i], outVals[mainCirc.nets[b].source.instIndex]);
				else
					printf("%s=%d ", mainCirc.outputs[i], outVals[mainCirc.nets[b].source.instIndex]);
			}
		}
		
	}
	free(outVals);
	return 0;
}
