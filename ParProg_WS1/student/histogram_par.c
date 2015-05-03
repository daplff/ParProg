#include <pthread.h>
#include <stdlib.h>
//#include <stdio.h>
#include "histogram.h"

//struct IOStructure {
//	unsigned int nBlocks;
//	block_t *blocks;
//	histogram_t histogram;
//} ;

struct IOStructureZ {
	pthread_t pid;
	unsigned int nBlocks;
	block_t *blocks;
	histogram_t histogram;
	unsigned short num_threads;
} ;
//
//void * wrap_seq_function(void * input);
//void get_histogram_seq(unsigned int nBlocks,block_t* blocks, unsigned int* histogram);
void * binary_fan(void* input);

void get_histogram(unsigned int nBlocks,
		   						 block_t *blocks,
		   						 histogram_t histogram,
		   						 unsigned int num_threads) 
{
	struct IOStructureZ * result = calloc(1,sizeof(*result));
	
	result->nBlocks=nBlocks;
	result->blocks=blocks;
	result->num_threads=num_threads;
	
	pthread_create(&(result->pid), NULL, &binary_fan, result);
	unsigned char i = 0;
	pthread_join(result->pid,NULL);
	for (; i<NALPHABET;++i)
	{
		histogram[i]=result->histogram[i];
	}
	free(result);
	
	/*pthread_t *threads = malloc(num_threads * sizeof(*threads));
	struct IOStructure ** inouts = calloc (num_threads, sizeof(*inouts)); //calloc initialises histogram to 0
	
	unsigned int numBase = nBlocks/num_threads;
	unsigned int numAdd = nBlocks%num_threads;
	unsigned int index = 0;
	for (;index<num_threads;++index)
	{
			inouts[index]=calloc(1,sizeof(*inouts[index]));
			inouts[index]->nBlocks=numBase + (index<numAdd ? 1 : 0);
			inouts[index]->blocks=blocks;
			blocks += inouts[index]->nBlocks;
			pthread_create(threads+index, NULL, &wrap_seq_function, inouts[index]);			
	}
	//TODO: parallel fan-in
	
	for (index=0;index<num_threads;++index)
	{
		pthread_join(threads[index],NULL);
		unsigned int j=0;
		unsigned int * histptr = inouts[index]->histogram;
		for(;j<NALPHABET;++j){
			histogram[j] +=  histptr[j];	
		}
		free(inouts[index]);
	}
	free(threads);
	free(inouts);
	
	
	*/
}
/*
void * wrap_seq_function (void * input)
{
	struct IOStructure* inout = input;
	get_histogram_seq(inout->nBlocks,inout->blocks,inout->histogram);
//	printf("this is a robbery. just kidding. this is a thread with %d blocks\n", inout->nBlocks);
	return NULL;
}

*/
void get_histogram_seq(unsigned int nBlocks,
		   						 block_t *blocks,
		   						 unsigned int* histogram) {

	unsigned int i, j;
//	for (i=0;i<NALPHABET;++i){
//		printf("init %c: %d\n",i + 'A',histogram[i]);
//	}
	// build histogram
	for (i=0; i<nBlocks; i++) {
		for (j=0; j<BLOCKSIZE; j++) {
			if (blocks[i][j] >= 'a' && blocks[i][j] <= 'z')
				histogram[blocks[i][j]-'a']++;
			else if(blocks[i][j] >= 'A' && blocks[i][j] <= 'Z')
				histogram[blocks[i][j]-'A']++;
		}
	}
}

void * binary_fan(void * input)
{
	struct IOStructureZ* inout = input;
	struct IOStructureZ* next = NULL;
	
	if(inout->num_threads > 1)
	{
		next = calloc(1,sizeof(*next));	//calloc IS VERY DANGEROUS ON NUMA SYSTEMS!!!!
//		if (next == NULL)
//			printf("ERROOOOOOOOOOOOOOOOOOOORR\n");
		next->num_threads = inout->num_threads/2;
		inout->num_threads -= next->num_threads;
		
		next->nBlocks = (inout->nBlocks - inout->nBlocks/2);
		inout->nBlocks -= next->nBlocks;
		
		next->blocks = inout->blocks + inout->nBlocks;
		
		pthread_create(&(next->pid),NULL,&binary_fan,next);
//		printf("fanning out with nBlocks=%d\t,pid=%ld\n",next->nBlocks,(long int)next->pid);
	}


	if(inout->num_threads > 1)
	{
//		printf("fanning out with nBlocks=%d\t,pid=%ld\n",inout->nBlocks,(long int)inout->pid);
		binary_fan(inout);
	}
	else
	{
//		printf("running with nBlocks=%d\t,pid=%ld\n",inout->nBlocks,(long int)inout->pid);
		get_histogram_seq(inout->nBlocks,inout->blocks,inout->histogram);
	}
	
	if(next!=NULL)
	{
		unsigned char i = 0;
		pthread_join(next->pid,NULL);
		for (;i<NALPHABET;++i)
		{
			inout->histogram[i]+=next->histogram[i];
		}
//		printf("closing pid=%ld\n",(long int)next->pid);
		free(next);
	}
	return NULL;
}