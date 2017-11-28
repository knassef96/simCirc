#include "circuit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void skipWhiteSpace(FILE *f);
char * nextName(FILE *f);

struct net * getNet(struct circuit *circ,char * name);
void addNetSource(struct net * net,int instIndex,int pinIndex);
void addNetSink(struct net * net,int instIndex,int pinIndex);
int isWhiteSpace(char c);

void printNet(struct circuit circ,int ni);
void printInstance(struct circuit circ, int ii);

struct circuit readCircuit(char * circuitName) {
	char filename[256];
	strcpy(filename,circuitName);
	strcat(filename,".txt");
	FILE *cf=fopen(filename,"r");
	if (cf==NULL) {
		printf("Unable to open circuit file: %s\n",filename);
		exit(1);
	}
	struct circuit circ;
	circ.numOutputs=0;
	char * oPin;
	circ.numNets=0;
	while(NULL != (oPin=nextName(cf))) {
		circ.outputs[circ.numOutputs]=oPin;
		struct net* net=getNet(&circ,oPin);
		addNetSink(net,-1,circ.numOutputs);
		circ.numOutputs++;
		assert(circ.numOutputs < MAXPINS);
	}
	int c=getc(cf);
	assert(c=='=');
	circ.name=nextName(cf);
	c=getc(cf);
	assert(c=='(');
	char *iPin;
	circ.numInputs=0;
	while(NULL != (iPin=nextName(cf))) {
			circ.inputs[circ.numInputs]=iPin;
			struct net* net=getNet(&circ,iPin);
			addNetSource(net,-1,circ.numInputs);
			circ.numInputs++;
			assert(circ.numInputs<MAXPINS);
	}
	c=getc(cf);
	assert(c==')');
	skipWhiteSpace(cf);
	c=getc(cf);
	assert(c=='{');
	// Process instances
	circ.numInstances=0;

	while(1) {
		int ii=circ.numInstances;
		struct instance *inst=&(circ.instances[ii]);
		// First, the list of outputs for this instance
		int nout=0;
		while(NULL != (oPin=nextName(cf))) {
			struct net* net=getNet(&circ,oPin);
			addNetSource(net,ii,nout);
			nout++;
			assert(nout<MAXPINS);
		}
		c=getc(cf);
		assert(c=='=');
		inst->name=nextName(cf);
		assert(0!=strcmp(inst->name,""));
		inst->moduleName=nextName(cf);
		assert(0!=strcmp(inst->moduleName,""));
		inst->numOutputs=nout;
		c=getc(cf);
		assert(c=='(');
		int nin=0;
		while(NULL != (iPin=nextName(cf))) {
			struct net* net=getNet(&circ,iPin);
			addNetSink(net,ii,nin);
			nin++;
			assert(nin<MAXPINS);
		}
		inst->numInputs=nin;
		c=getc(cf);
		assert(c==')');
		circ.numInstances++;
		assert(circ.numInstances<MAXINST);
		skipWhiteSpace(cf);
		c=getc(cf);
		if (c=='}') break; // Done with all instances
		ungetc(c,cf);
	}

	fclose(cf);
	return circ;
}

void printCircuit(struct circuit circ) {
	printf("- name-> \"%s\" numInputs=%d numOutputs=%d numNets=%d numInstances=%d\n",
		circ.name,circ.numInputs,circ.numOutputs,circ.numNets,circ.numInstances);

	int i;
	printf("- inputs={");
	for(i=0;i<circ.numInputs;i++) {
		printf(" \"%s\"",circ.inputs[i]);
		if (i<circ.numInputs-1) printf(",");
	}
	printf(" }\n- outputs={");
	for(i=0;i<circ.numOutputs;i++) {
		printf(" \"%s\"",circ.outputs[i]);
		if (i<circ.numOutputs-1) printf(",");
	}
	printf(" }\n");
	for(i=0;i<circ.numNets;i++) {
		printNet(circ,i);
	}
	for(i=0;i<circ.numInstances;i++) {
		printInstance(circ,i);
	}
}

void printNet(struct circuit circ,int ni) {
	printf("- nets[%d] name->\"%s\" numSinks=%d source { instIndex=%d, instPin=%d }\n",
		ni,circ.nets[ni].name,circ.nets[ni].numSinks,
		circ.nets[ni].source.instIndex, circ.nets[ni].source.pinIndex);
	int i;
	for(i=0;i<circ.nets[ni].numSinks; i++) {
		printf("   - sinks[%d] { instIndex=%d, pinIndex=%d }\n",
			i,circ.nets[ni].sinks[i].instIndex, circ.nets[ni].sinks[i].pinIndex);
	}
}

void printInstance(struct circuit circ, int ii) {
	printf("- instance[%d] { name->\"%s\" moduleName->\"%s\", numInputs=%d numOutputs=%d }\n",
		ii,circ.instances[ii].name,circ.instances[ii].moduleName,
		circ.instances[ii].numInputs,circ.instances[ii].numOutputs);
}

void skipWhiteSpace(FILE *f) {
	int c;
	do {
		c = getc(f);
	} while(isWhiteSpace(c));
	ungetc(c,f);
}

int isWhiteSpace(char c) {
	return (c==' ' || c=='\t' || c=='\n' || c=='\r');
}

char * nextName(FILE *f) {
	int i=0; int c; char buff[32];
	skipWhiteSpace(f);
	while(1) {
		c=getc(f);
		if (c==',') break;
		if (c=='=') break;
		if (c=='(') break;
		if (c==')') break;
		if (isWhiteSpace(c)) break;
		buff[i]=c;
		i++;
	}
	buff[i]=0x00; // Add null terminator
	if (c != ',') ungetc(c,f); // Skip commas
	if (i==0) return NULL;
	skipWhiteSpace(f);
	return strdup(buff);
}

struct net* getNet(struct circuit *circ,char * name) {
	int ni=circ->numNets;
	int i;
	for(i=0; i<ni; i++) {
		if (0==strcmp(circ->nets[i].name,name)) return &(circ->nets[i]);
	}
	circ->nets[ni].name=name;
	circ->nets[ni].source.instIndex=-2; // Flag source not set
	circ->nets[ni].numSinks=0;
	circ->numNets++;
	assert(circ->numNets<MAXNETS);
	return &(circ->nets[ni]);
}

int netIndex(struct circuit circ, char * netName) {
	int i;
	for(i=0; i<circ.numNets; i++) {
		if (0==strcmp(circ.nets[i].name,netName)) return i;
	}
	printf("Net %s not found in circuit\n",netName);
	assert(0);
}

void addNetSource(struct net * net,int instIndex,int pinIndex) {
	net->source.instIndex=instIndex;
	net->source.pinIndex=pinIndex;
}

void addNetSink(struct net * net,int instIndex,int pinIndex) {
	int ns=net->numSinks;
	net->sinks[ns].instIndex=instIndex;
	net->sinks[ns].pinIndex=pinIndex;
	net->numSinks++;
	assert(net->numSinks<MAXSINKS);
}

void freeCircuit(struct circuit circ) {
	free(circ.name);
	int i;
	for(i=0;i<circ.numOutputs;i++) {
		free(circ.outputs[i]);
	}
	for(i=0;i<circ.numInputs;i++) {
		free(circ.inputs[i]);
	}
	for(i=0;i<circ.numInstances;i++) {
		free(circ.instances[i].name);
		free(circ.instances[i].moduleName);
	}
}
