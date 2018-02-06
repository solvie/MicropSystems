#include "gpio.h"
#include "lis3dsh.h"
#include "accelerometer.h"
void Read_ACC_Value(void);

uint8_t status;
float Buffer[3];
float accX, accY, accZ;
extern int acc_flag;
extern int read_flag;

typedef struct{
  float a[3];
  float b[3];
}FIR_coeff;

int data_ready = 0;
const int BUFFER_SIZE = 4;
int buffer_counter = 0;
int filtered_counter = 0;
float x_buffer[BUFFER_SIZE];
float y_buffer[BUFFER_SIZE];
float z_buffer[BUFFER_SIZE];

float x_filtered[BUFFER_SIZE];
float y_filtered[BUFFER_SIZE];
float z_filtered[BUFFER_SIZE];

FIR_coeff coeffStruct_x = {{1.0000,-0.2430,0.2951}, {0.2427,0.4853,0.2427}};
FIR_coeff coeffStruct_y = {{1.0000,-0.2386,0.2938}, {0.2437,0.4873,0.2437}};
FIR_coeff coeffStruct_z = {{1.0000,-0.1961,0.2826}, {0.2535,0.5070,0.2535}};

float Cal_M[4][3] = {
	{0.0010,-0.0000,-0.0000},
	{-0.0000,0.0010,-0.0000},
	{0.0000,0.0000,0.0010},
	{0.0025,-0.0074,-0.0129}
};

/**
* Interrupt handler for the accelerometer interrupt received at PE0.
* @param GPIO_Pin where the interrupt is read
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_0){
		acc_flag = 1;
		Read_ACC_Value();
	}
}

/**
* Helper for Apply_IIR_Filter that returns the input array @param array's value at index @param n 
* if n is not negative, and 0 otherwise. 
*/
float zeroOrArrayPos(int n, float* array ){
	if(n<0) return 0;
	else return *(array+n);
}

/**
* Takes the calibrated values from @param InputArray, applies the IIR filter to it,
* using coeffecients relevant for the axis input @param axis (which can be x, y, or z)
* and writes the result to the from @param OutputArray
*/
float Apply_IIR_Filter(float* InputArray, float* OutputArray, char axis){
	FIR_coeff* coeff;
	if(axis == 'z'){
		coeff = &coeffStruct_z;
	}else if(axis == 'y'){
		coeff = &coeffStruct_y;
	}else{
		coeff = &coeffStruct_x;
	}
	
	int Length = BUFFER_SIZE;
	int Order = 2;
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

/**
* Reads the raw values from the LIS3DSH accelerometer and sets the variables accX, accY, accZ
*/
void Read_ACC_Value(){
	LIS3DSH_Read (&status, LIS3DSH_STATUS, 1);
	//The first four bits denote if we have new data on all XYZ axes, 
	//Z axis only, Y axis only or Z axis only. If any or all changed, proceed
	if ((status & 0x0F) != 0x00)
	{
		LIS3DSH_ReadACC(&Buffer[0]);
		accX = (float)Buffer[0];
		accY = (float)Buffer[1];
		accZ = (float)Buffer[2];
	}
}

/**
* Calibrates the accelerometer values
*/
void ACC_Read_Value(void){
	float new_value[3];
	new_value[0] = accX * Cal_M[0][0] + accY * Cal_M[1][0] + accZ * Cal_M[2][0] + Cal_M[3][0];
	new_value[1] = accX * Cal_M[0][1] + accY * Cal_M[1][1] + accZ * Cal_M[2][1] + Cal_M[3][1];
	new_value[2] = accX * Cal_M[0][2] + accY * Cal_M[1][2] + accZ * Cal_M[2][2] + Cal_M[3][2];	

	x_buffer[buffer_counter] = new_value[0];
	y_buffer[buffer_counter] = new_value[1];
	z_buffer[buffer_counter] = new_value[2];

	buffer_counter += 1;
	if(buffer_counter == BUFFER_SIZE){
		read_flag = 1;	
	}
}

/**
* Reads the filtered, calibrated accelerometer values and updates the @param value array accordingly.
*/
void Read_ACC(float *value){
	Apply_IIR_Filter(&x_buffer[0], &x_filtered[0], 'x');
	Apply_IIR_Filter(&y_buffer[0], &y_filtered[0], 'y');
	Apply_IIR_Filter(&z_buffer[0], &z_filtered[0], 'z');
	
	value[0] = x_filtered[BUFFER_SIZE-1];
	value[1] = y_filtered[BUFFER_SIZE-1];
	value[2] = z_filtered[BUFFER_SIZE-1];
	buffer_counter = 0;
}
