#include "gpio.h"
#include "display.h"
#include "math.h"


//needs to throw exception when 2 digits or more before decimal point
void floatTo4DigitArray(int* digitArray, float fVal)  
{    fVal += 0.0005; //So that we round up properly
    digitArray[0] = (int) fVal;
    fVal = (fVal-digitArray[0]) * 10;
    digitArray[1] = (int) fVal;
    fVal = (fVal-digitArray[1]) * 10;
    digitArray[2] = (int) fVal;
    fVal = (fVal-digitArray[2]) * 10;
    digitArray[3] = (int) fVal;
}

/**
* input number must be 1~ 3 digits long. 
*/
void intToArray(int * array, int number){
		const int NUM_LED_DIGITS = 4;
    int length = log10(number) + 1;
    int i;
		for (i =0; i<NUM_LED_DIGITS-length; i++)
			array[i] = -1;
    for ( i = 0; i < length; ++i, number/=10)
			array[NUM_LED_DIGITS-1-i] = number % 10;
}

/*
A - 7
B - 8
C - 9
D - 10 
E - 11
F - 12
G - 13
*/
void displayInt(int toDisplay){
	if (toDisplay==0){ //G off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);
	}
	if (toDisplay==1){ //B and E on
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);
	}
	if (toDisplay==2){ //F and C off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}	if (toDisplay==3){ // F and E off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}	if (toDisplay==4){ //A, D, and E off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}	if (toDisplay==5){ //B and E off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}	if (toDisplay==6){// B off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}	if (toDisplay==7){ //A, B, C on
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);
	}	
	if (toDisplay==8){  //All segments on
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}
		if (toDisplay==9){  // E off
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}
}

void digitSelect(int* digitArray,int currentDigit ){
		if (currentDigit==0){ 
			if (digitArray[0]>=0){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
				displayInt(digitArray[0]);
			}else {
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			}
		}
		
		if (currentDigit==1){ 
			if (digitArray[1]>=0){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
				displayInt(digitArray[1]);
			}else {
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			}

		}if (currentDigit==2){ 
			if (digitArray[2]>=0){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
				displayInt(digitArray[2]);
			} else{
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			}

		}if (currentDigit==3){ 
			if (digitArray[3]>=0){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET);
				displayInt(digitArray[3]);
			} else {
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			}
		}

}