//#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "lis3dsh.h"
#include "accelerometer.h"
void Read_ACC_Value(void);
uint8_t status;
float Buffer[3];
float accX, accY, accZ;
extern int acc_flag;
typedef struct{
  float a[3];
  float b[3];
}FIR_coeff;


FIR_coeff coeffStruct = {{0, 0.3,-0.2}, {0.2,0.4,0.2}};
float Cal_M[4][3] = {
	{0.0010,-0.0000,-0.0000},
	{-0.0000,0.0010,-0.0000},
	{0.0000,0.0000,0.0010},
	{0.0025,-0.0074,-0.0129}
};


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
		if(GPIO_Pin == GPIO_PIN_0){
			acc_flag = 1;
			Read_ACC_Value();
			
		}
	}

float IIR_C(float* InputArray, float* OutputArray,FIR_coeff* coeff, int Length, int Order){
	float tempY;
	for (int i=0; i<Length;i++){
		tempY = (coeff->b)[0]*InputArray[i];
		for (int j=1; j<Order+1;j++){		
			if(i>=j)
				tempY = tempY 
						+(coeff->b)[j]*InputArray[i-j]
							+(coeff->a)[j]*OutputArray[i-j];
		}
		*(OutputArray+i)=tempY; //set the value at index i of OutputArray to the tempY that was just found.

	}
	return tempY;
}

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
		//printf("%3f,%3f,%3f\n",accX, accY, accZ);
		
	}
}

void Calibrate_ACC_Value(float *value){
		float new_value[3];
		new_value[0] = accX * Cal_M[0][0] + accY * Cal_M[1][0] + accZ * Cal_M[2][0] + Cal_M[3][0];
		new_value[1] = accX * Cal_M[0][1] + accY * Cal_M[1][1] + accZ * Cal_M[2][1] + Cal_M[3][1];
		new_value[2] = accX * Cal_M[0][2] + accY * Cal_M[1][2] + accZ * Cal_M[2][2] + Cal_M[3][2];	
		//printf("Temp: X: %3f   Y: %3f   Z: %3f \n",new_value[0], new_value[1], new_value[2]);
		value[0] = new_value[0];
		value[1] = new_value[1];
		value[2] = new_value[2];
}
