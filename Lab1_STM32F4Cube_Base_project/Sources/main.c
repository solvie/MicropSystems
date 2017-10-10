#include <stdio.h>
//#include "arm_math.h"

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
		for (int j=1; j<Order+1;j++)
			tempY = tempY 
						+(coeff->b)[j]*zeroOrArrayPos(i-j,InputArray)
							+(coeff->a)[j]*zeroOrArrayPos(i-j,OutputArray);
		*(OutputArray+i)=tempY; //set the value at index i of OutputArray to the tempY that was just found.

	}
	return tempY;
}

int main(){
	int inputLength=5;
	int inputOrder=2;

	//initialize input and output
	float inputExampleC[5] = {1,1,1,1,1};
	float outputExampleC[5] = {0,0,0,0,0};
	float outputExampleAsm[5] = {0,0,0,0,0};

	FIR_coeff coeffStruct = {{0, 1,1}, {1, 1,1}};

	//call C code
	float out = IIR_C(&inputExampleC[0], &outputExampleC[0], &coeffStruct, inputLength, inputOrder);
	printf("C result\n\n:");
	for (int i=0; i<5; i++){
	   printf("%f ", outputExampleC[i]);
	}

	//call assembly code/
	IIR_asm(&inputExampleC[0],&outputExampleAsm[0], inputLength, &coeffStruct);
	printf("\n\nAssembly result:\n\n");
	for (int i=0; i<5; i++){
	    printf("%f ", outputExampleAsm[i]);
	}
	
	return 0;
	
}
