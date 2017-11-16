#include "gpio.h"
#include "display.h"
#include "math.h"
#include "cmsis_os.h"  
#include "tim.h"

#define SIGNAL_READY 0x0001
#define SIGNAL_WAIT 0x0002
#define PI 3.14159265
/**
* Converts an integer @param number (which must be 1~3 digits long) into an array of integers @param array.
* An array value of -1 indicates that it is empty. 
*/
float acc_value_global[3];
int currentDigit = 0;
int sleep_flag;
int operation_flag;
int reset_flag;
char key_pressed_global = 0;
int entered_char_pointer = 4;
int sleepmode=0;
int inputRollExpected = 0;
int inputPitchExpected = 0;
int reinit=0;
int toDisplay=0;
int digitArray[4];
int userInputState;
int enterRollState; 
int operatingModeRollMonitoring; //the first operating mode we enter will be roll monitoring

extern TIM_HandleTypeDef htim2;
osThreadDef(Display_Thread, osPriorityNormal, 1, 0);
osThreadId Display_Thread_ID;

osSemaphoreId key_pressed_sem_id;                         // Semaphore ID
osSemaphoreDef(key_pressed_sem);                       // Semaphore definition

osSemaphoreId acc_ready_sem_id;                         // Semaphore ID
osSemaphoreDef(acc_ready_sem);                       // Semaphore definition

void intToArray(int * array, int number){
	const int NUM_LED_DIGITS = 4;
    int length = log10(number) + 1;
    int i;
	for (i =0; i<NUM_LED_DIGITS-length; i++)
		array[i] = -1;
    for ( i = 0; i < length; ++i, number/=10)
		array[NUM_LED_DIGITS-1-i] = number % 10;
}

/**
*	Sets the relevant segments on the 7-segment to display the digit which is to be displalyed, 
* 	@param toDisplay, to high or low.
*	Segments to pin number mapping:
*	A - 7
*	B - 8
*	C - 9
*	D - 10 
*	E - 11
*	F - 12
*	G - 13
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
	}	if (toDisplay==1){ //B and E on
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);
	}	if (toDisplay==2){ //F and C off
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
	}	if (toDisplay==8){  //All segments on
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}	if (toDisplay==9){  // E off
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET);
	}
}

/**
* Resets all of the 7-segment display pins, to shut the display off.
*/
void resetAll(){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
	
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_7,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_8,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_9,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_11,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET);
}

/**
* From the @param digitArray, if the array's value at @param currentDigit's index is not -1, we enable the 
* digit on the 7-segment display. If the value at the relevant index is -1, we disable the digit, not displaying it. 
*/
void digitSelect(int* digitArray,int currentDigit){
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
int toggleDigit(){
	currentDigit=(currentDigit+1)%4;
	return currentDigit;
}


void set_acc_value_display(float *temp_value){
	osSemaphoreWait(acc_ready_sem_id,osWaitForever);
	acc_value_global[0] = temp_value[0];
	acc_value_global[1] = temp_value[1];
	acc_value_global[2] = temp_value[2];
	
	osSemaphoreRelease(acc_ready_sem_id);
}

void get_acc_value_display(float *temp_value){
	osSemaphoreWait(acc_ready_sem_id,osWaitForever);
	temp_value[0] = acc_value_global[0];
	temp_value[1] = acc_value_global[1];
	temp_value[2] = acc_value_global[2];
	printf("X: %f, Y: %f, Z: %f", temp_value[0], temp_value[1], temp_value[2]);
	osSemaphoreRelease(acc_ready_sem_id);
}

void set_key_pressed_display(char c){
	osSemaphoreWait(key_pressed_sem_id,osWaitForever);
	key_pressed_global = c;
	osSemaphoreRelease(key_pressed_sem_id);
}

char get_key_pressed_display(void){
	char temp;
	osSemaphoreWait(key_pressed_sem_id,osWaitForever);
	temp = key_pressed_global;
	osSemaphoreRelease(key_pressed_sem_id);
	return temp;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{		if(htim == &htim2){
			osSignalSet(Display_Thread_ID, SIGNAL_READY);
		}
}
void Display_Thread(void const *argument){
	const int DISPLAY_COUNTER_MAX = 5; 
	int displayCounter = 0;
	float acc_value[3];
	
	char key_pressed;
	userInputState=1;
	enterRollState=1; 
	operatingModeRollMonitoring=1;
	while(1){
		
		osSignalWait(SIGNAL_READY, osWaitForever);
		key_pressed = get_key_pressed_display();
		set_key_pressed_display('\0');
		if(key_pressed != '\0'){
			printf("Key is %c \n", key_pressed);
		}
		if (!sleepmode){
			HAL_TIM_Base_Start_IT(&htim2);//DOUBLE CHECK!!
			if (userInputState){
				if (entered_char_pointer==4){//nothing has been entered yet
					initializeDisplayToZero();//display zero if nothing has been entered
				}
				if(key_pressed != '\0'&&key_pressed != '*'&&key_pressed != '#'){ //a number was entered
					printf("Key Pressed is %c \n", key_pressed);
					enterNumberIntoBuffer(key_pressed - '0');
				} else if(key_pressed == '#') {
					int concatedint = concatenateArray();// concatenate array contents into one integer/.
					printf("Input is %d \n", concatedint);
					if (enterRollState){ 
						inputRollExpected = concatedint;
						if(!reinit){//If not reinitializing, go to enterPitchState
							enterRollState=0;
							entered_char_pointer=4;
							initializeDisplayToZero();
							user_pwm_set_led_brightness(0,500,500,0);
						} else{ //if reinitializing, go directly back to roll monitoring operating mode
							reinit=0; 
							userInputState=0; //operating mode
							operatingModeRollMonitoring=1;
						  //enterRollState=1;
						}
					}	else{ //Is enterPitchState
						inputPitchExpected = concatedint;
						if(!reinit){//If not reinitializing, roll monitoring operating mode
							userInputState=0;
							operatingModeRollMonitoring=1;
						} else{//if reinitializing, go directly back to pitch monitoring operating mode
							reinit=0;
							userInputState=0;
							operatingModeRollMonitoring=0;
						}
					}
				} else if (key_pressed=='*'){
					printf("Key Pressed is %c \n", key_pressed);
					deleteLastInBuffer();
				}
			}else{ // Is in operatingMode
			//Update the value to be shown in 7-segment display.
				get_acc_value_display(&acc_value[0]);
				if (operatingModeRollMonitoring){
					printf("okkkk");
					//adjustBrightnessBasedOnACC(0, inputRollExpected, &acc_value[0]);
				} else{//if not in operatingModeRollMonitoring is operatingModePitchMonitoring
					//adjustBrightnessBasedOnACC(1, inputPitchExpected, &acc_value[0]);
					printf("NONON");
				}

			
				if(key_pressed == '1'){ //go to roll
					printf("Key Pressed is %c \n", key_pressed);
					operatingModeRollMonitoring=1;
				} else if(key_pressed == '2'){ //go to pitch
					printf("Key Pressed is %c \n", key_pressed);
					operatingModeRollMonitoring=0;
				} if (sleep_flag){//enter sleep mode
					sleep_flag=0;
					sleepmode=1;
					resetAll(); 
					HAL_TIM_Base_Stop_IT(&htim2);
					for (int i=0; i<4; i++)digitArray[i] = -1;
					//user_pwm_set_led_brightness(0,0,0,0);//Dim the LEDS
				} else if (reset_flag){ //go to input mode for the relevant roll state
					reset_flag=0;
					userInputState=1;
					reinit = 1;
					if (operatingModeRollMonitoring){
						enterRollState=1;
						entered_char_pointer=4;
						initializeDisplayToZero();
						//user_pwm_set_led_brightness(500,0,0,500);
					} else{
						enterRollState=0;
						entered_char_pointer=4;
						initializeDisplayToZero();
						//user_pwm_set_led_brightness(0,500,500,0);
					}
				}
				if (displayCounter==DISPLAY_COUNTER_MAX-1){ //Waiting for counter to reach 99 ensures display is updated less frequently than interrupt rate from timer (so as changes to be easily visible)
					intToArray(&digitArray[0],toDisplay);
				}
			} 
		} else{ //Is in sleep mode
			resetAll(); 
		  
			if(operation_flag){
				sleepmode=0;
				userInputState=0;
				operation_flag=0;
			}
		}
		displayCounter = (displayCounter+1)%DISPLAY_COUNTER_MAX;
}
}
void initializeDisplayToZero(){
	int i;
	for (i=0; i<3; i++) digitArray[i] = -1;
	digitArray[3] = 0; //display zero if nothing has been entered
}

int concatenateArray(){
	if (entered_char_pointer==4) entered_char_pointer=3;//If a user input nothing, set the pointer to the least significant digit, aka the last array index, where the zero is.
	int concatedint = digitArray[entered_char_pointer];
	for (int i = entered_char_pointer+1; i<4; i++) concatedint = concatenate(concatedint,digitArray[i] );
	return concatedint;
}

int concatenate(int x, int y) {
    int pow = 10;
    while(y >= pow) pow *= 10;
    return x * pow + y;        
}

void enterNumberIntoBuffer(int numberEntered){
	if (entered_char_pointer>1){ //as long as three digits haven't already been filled, a user can type a new letter
		for (int i=entered_char_pointer; i<4; i++){
			digitArray[i-1] =digitArray[i]; //shift what was already there
		}
		digitArray[3] = numberEntered; //add the pressed key to the last element.
		entered_char_pointer--;
	}
}

void deleteLastInBuffer(){
		const int lastindex = 3; //last of indices 0,1,2,3
		if (entered_char_pointer<4){ //as long as three digits haven't already been filled, a user can type a new letter
			if (entered_char_pointer==3)
				digitArray[3] = 0;
			else{ //entered_char_pointer= 2 or 1 
				int i;
				for (i=lastindex-1; i>entered_char_pointer-1; i--){
					digitArray[i+1]=digitArray[i]; //shift what was already there
				}
				digitArray[i+1]=-1;//set the previously highest bit to empty
			}
			entered_char_pointer++;
		}
}

void adjustBrightnessBasedOnACC(int isPitch, float expectedPitchOrRoll, float* valsFromAcc){
	uint16_t diffMagnitudeForBrightness;
	float calculated, convertedTo180scale, diff;
	float ax = valsFromAcc[0]; //doing absolute values for now.
	float ay = valsFromAcc[1];
	float az = valsFromAcc[2];
	if (isPitch){
		calculated = calculatePitchAngleFromAccVals(ax, ay, az);
		convertedTo180scale= calculated+90.5;//adding .5 to cast to int properly
		// The diffMagnitudeForBrightness will be from 0 to 90 (because of abs). 
		diff = expectedPitchOrRoll- convertedTo180scale;
		diffMagnitudeForBrightness= (uint16_t) fabs( expectedPitchOrRoll - convertedTo180scale);
	
		if (diff>0)
			user_pwm_set_led_brightness(0,0,diffMagnitudeForBrightness * 5.555555,0); //5.5555 = 500/90
		else
			user_pwm_set_led_brightness(0,diffMagnitudeForBrightness * 5.555555,0,0); //5.5555 = 500/90
	}
	else {
		calculated = calculateRollAngleFromAccVals(ax, ay, az);
		convertedTo180scale= calculated+90.5;//adding .5 to cast to int properly
		diff = expectedPitchOrRoll- convertedTo180scale;
		// The diffMagnitudeForBrightness will be from 0 to 90 (because of abs).
		diffMagnitudeForBrightness=  (uint16_t)fabs( expectedPitchOrRoll - convertedTo180scale);
		if (diff>0)
			user_pwm_set_led_brightness(0,0,0,diffMagnitudeForBrightness * 5.555555); //5.5555 = 500/90
		else
			user_pwm_set_led_brightness(diffMagnitudeForBrightness * 5.555555,0,0,0); //5.5555 = 500/90
	}
	toDisplay= (int)convertedTo180scale;
}

float calculatePitchAngleFromAccVals(float ax, float ay, float az){
	float val = 180.0 / PI;
	float retval;
	float denom = ay*ay+az*az;
	denom = sqrt(denom);
	retval = ax/denom;
	retval = atan(retval)*val;
	return retval;
}

float calculateRollAngleFromAccVals(float ax, float ay, float az){
	float val = 180.0 / PI;
	float retval;
	float denom = ax*ax+az*az;
	denom = sqrt(denom);
	retval = (-ay)/denom;
	retval = atan(retval)*val;
	return retval;
}

void user_pwm_set_led_brightness(uint16_t ld3, uint16_t ld4,uint16_t ld5,uint16_t ld6){
	TIM_OC_InitTypeDef sConfigOC;
  
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	  
	sConfigOC.Pulse = ld4;
    HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
	sConfigOC.Pulse = ld3;
	HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2);
	sConfigOC.Pulse = ld5;
	HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3);
	sConfigOC.Pulse = ld6;
    HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);

    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);  
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);  
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);  
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);  
}

void start_display_thread(void){
	Display_Thread_ID = osThreadCreate(osThread(Display_Thread), NULL); 
	osSignalSet(Display_Thread_ID, SIGNAL_WAIT);
	key_pressed_sem_id = osSemaphoreCreate(osSemaphore(key_pressed_sem), 1); 
	acc_ready_sem_id = osSemaphoreCreate(osSemaphore(acc_ready_sem), 1);
}