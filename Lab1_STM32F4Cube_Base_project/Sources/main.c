#include <stdio.h>
#include "arm_math.h"

typedef struct{
  float a[3];
  float b[3];
}FIR_coeff;

extern int IIR_asm(float* a, float* b, int c, FIR_coeff* d);

//If the array index is less than 0, returns 0. Otherwise, return the value of the array at index n.
float zeroOrArrayPos(int n, float* array ){
	if(n<0) return 0;
	else return *(array+n);
}

//Version with all coefficients set as 1
float IIR_C(float* InputArray, float* OutputArray,FIR_coeff* coeff, int Length, int Order){
	float tempY;
	for (int i=0; i<Length;i++){

//for testing
		tempY = (coeff->b)[0]*zeroOrArrayPos(i,InputArray);
		for (int j=1; j<Order+1;j++)			tempY = tempY 
						+(coeff->b)[j]*zeroOrArrayPos(i-j,InputArray)
							+(coeff->a)[j]*zeroOrArrayPos(i-j,OutputArray);
		*(OutputArray+i)=tempY; //set the value at index i of OutputArray to the tempY that was just found.

	}
	return tempY;
}

void IIR_CMSIS(arm_biquad_casd_df1_inst_f32 *S, float32_t *pSrc, float32_t *pDst,  float32_t *pCoeffs,float32_t *pState, uint8_t numStages, uint32_t blockSize){
	arm_biquad_cascade_df1_init_f32(S,numStages,pCoeffs,pState);
  arm_biquad_cascade_df1_f32(S,	pSrc, pDst, blockSize);	
}

int main(){
	arm_biquad_casd_df1_inst_f32 S;
	int inputLength=5;
	int inputOrder=2;

	//initialize input and output
	float inputExampleC[5] = {1,1,1,1,1};
	float inputExampleAsm[5] = {1,1,1,1,1};
	float inputExampleCMSIS[5] = {1,1,1,1,1};

	float outputExampleC[5] = {0,0,0,0,0};
	float outputExampleAsm[5] = {0,0,0,0,0};
	float outputExampleCMSIS[5] = {0,0,0,0,0};

	uint8_t numstages=1;
	uint32_t blockSize=5;

  float32_t pState[1*4]={0}; //numstages*4
	FIR_coeff coeffStruct = {{0, 0.3,-0.2}, {0.2,0.4,0.2}};
	float coeffForCMSIS[5] = {0.2,0.4,0.2, 0.3,-0.2};

	//call C code
	IIR_C(&inputExampleC[0], &outputExampleC[0], &coeffStruct, inputLength, inputOrder);
	printf("C result:\n\n");
	for (int i=0; i<5; i++){
	   printf("%f ", outputExampleC[i]);
	}
	
	//call CMSIS-DSP code
	printf("\n\n");
	IIR_CMSIS(&S,inputExampleCMSIS, outputExampleCMSIS,coeffForCMSIS,pState,numstages,blockSize );
	printf("Library result:\n\n");
	for (int i=0; i<5; i++){
	    printf("%f ", outputExampleCMSIS[i]);
	}	
	printf("\n\n");

  //call assembly code/
	IIR_asm(&inputExampleAsm[0],&outputExampleAsm[0], inputLength, &coeffStruct);
	printf("Assembly result:\n\n");
	for (int i=0; i<5; i++){
	    printf("%f ", outputExampleAsm[i]);
	}	
	return 0;
	
}
