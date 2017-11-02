#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "lis3dsh.h"

void Read_ACC_Value();
uint8_t status;
float Buffer[3];
float accX, accY, accZ;
extern int acc_flag;

float Cal_M[4][3] = {
	{1,2,3},
	{4,5,6},
	{7,8,9},
	{10,11,12}
};


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
		if(GPIO_Pin == GPIO_PIN_0){
			acc_flag = 1;
			Read_ACC_Value();
			
		}
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
		printf("%3f,%3f,%3f\n",accX, accY, accZ);
		
	}
}

void Calibrate_ACC_Value(float *value){
		float new_value[3];
		new_value[0] = accX * Cal_M[0][0] + accY * Cal_M[1][0] + accZ * Cal_M[2][0] + Cal_M[3][0];
		new_value[1] = accX * Cal_M[0][1] + accY * Cal_M[1][1] + accZ * Cal_M[2][1] + Cal_M[3][1];
		new_value[2] = accX * Cal_M[0][2] + accY * Cal_M[1][2] + accZ * Cal_M[2][2] + Cal_M[3][2];	
		*value = *new_value;
}