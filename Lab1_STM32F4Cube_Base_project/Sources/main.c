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
float IIR_FCtemp(float* InputArray, float* OutputArray,FIR_coeff* coeff, int Length, int Order){
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
	printf("hello world\n\n");
	//initialize input and output
	float inputExample[5] = {1,1,1,1,1};
	float outputExample[5] = {0,0,0,0,0};
	
	float * inP;
	float * outP;

	inP=&inputExample[0];
	outP=&outputExample[0];
	FIR_coeff coeffStruct = {{0, 1,1}, {1, 1,1}};
	
	FIR_coeff* coeff = &coeffStruct;
	
	float out = IIR_FCtemp(inP, outP, coeff, 5, 2);
	printf("final result\n\n");
	
	//call assembly code
	//IIR_asm(inP,outP, array_length, coeff);
	for (int i=0; i<5; i++){
	    printf("%f ", outP[i]);
	}
	return 0;
	
}
