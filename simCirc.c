#include "circuit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


//Evaluates each of the 5 basic circuits
bool AND(bool i1, bool i2){return (i1&&i2);}
bool OR(bool i1, bool i2){return (i1||i2);}
bool INV(bool i1){return !i1;}
bool GND(){return 0;}
bool VDD(){return 1;}


//Returns true if string is one of the five basic circuits
bool isBasicCircuit(char *str){
	return (
		strcmp(str, "AND")==0 ||
		strcmp(str, "OR")==0  ||
		strcmp(str, "INV")==0 ||
		strcmp(str, "GND")==0 ||
		strcmp(str, "VDD")==0 );
}

//this will the the source value for the input of a gate instance, i 
// current is the current net to get teh values from
//	inputVals are the user inputted values 
//	outputVals are the current outputs of all the gate instances
char getSource(struct net current, int i, char *inputVals, char outVals[]) {
	if (current.source.instIndex == -1) 
		return inputVals[current.source.pinIndex];
	else 
		return outVals[i];
}

// TODO: this will wdo tha actual gate logic and store the output of the gate value into the outVals array
void evaluateBasic() {

}

int main(int argc, char *argv[]){
	//Check to make sure proper number of args
	if(argc != 3)
	{
		printf("Input Error: wrong number of args");
		exit(1);
	}
	//Declare circuits struct and input string
	struct circuit mainCirc;
	char *inputVals;

	//Read in information
	mainCirc = readCircuit(argv[1]);
	inputVals = argv[2];
	char outVals[mainCirc.numInstances]; // hold the output of the gate instances

	//Print the circuit name and each of the inputs
	printf("Simulated circuit %s\n", mainCirc.name);
	printf("Inputs: ");
	for(int i = 0; i<mainCirc.numInputs; i++){
		//If the last element, replace the space with a newline char
		if(i==mainCirc.numInputs-1)
			printf("%s=%c\n", mainCirc.inputs[i], inputVals[i]);
		else
			printf("%s=%c ", mainCirc.inputs[i], inputVals[i]);
	}
	for (int i = 0; i < mainCirc.instances; i++) {
		bool inputs[mainCirc.instances[i].numInputs];	//inputs for the instance gate;
		for (int j = 0; j < mainCirc.nets; j++) {		//loop through the nets and find the input values for the current instance
			struct net current = mainCirc.nets[j];		//current net
			for (int k = 0; k < current.numSinks; k++) {	//iterate through sinks to find the right one for the gate
				if (current.sinks[k].instIndex == i) {		//if there is a match then write to gate inputs;
					inputs[current.sinks[k].pinIndex] = getSource(current, i, inputVals, outVals);
				}	
			}
		}
		evaluateBasic();
	}
}
