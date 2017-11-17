#include "gpio.h"
#include "display.h"
#include "math.h"
#include "cmsis_os.h"  
#include "tim.h"
#include "accelerometer.h"


#define SIGNAL_READY 0x0001
#define SIGNAL_WAIT 0x0002

#define reset_flag_const 0
#define sleep_flag_const 1
#define operation_flag_const 2
/**
* Converts an integer @param number (which must be 1~3 digits long) into an array of integers @param array.
* An array value of -1 indicates that it is empty. 
*/
//float acc_value_global[3];
float pitch_roll_value_global[2];
const uint16_t EXCEEDS_MAX = 61568; //WARNING MUST BE GREATER THAN PULSE

int digitArray[4];

int currentDigit = 0;
int sleep_flag_global;
int operation_flag_global;
int reset_flag_global;
char key_pressed_global = 0;
int entered_char_pointer = 4;
int sleepmode=0;
int inputRollExpected = 0;
int inputPitchExpected = 0;
int reinit=0;
int toDisplay=0;
int userInputState;
int enterRollState; 
int operatingModeRollMonitoring; //the first operating mode we enter will be roll monitoring
int viewAccelVals;

extern TIM_HandleTypeDef htim2;

osThreadDef(Display_Thread, osPriorityNormal, 1, 0);
osThreadId Display_Thread_ID;

osSemaphoreId key_pressed_sem_id;                         // Semaphore ID
osSemaphoreDef(key_pressed_sem);                       // Semaphore definition

osSemaphoreId acc_ready_sem_id;                         // Semaphore ID
osSemaphoreDef(acc_ready_sem);                       // Semaphore definition

osSemaphoreId flag_sem_id;                         // Semaphore ID
osSemaphoreDef(flag_sem);                       // Semaphore definition

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
void disableAllGPIOClocks(){
  __GPIOE_CLK_DISABLE();
  __GPIOC_CLK_DISABLE();
  __GPIOH_CLK_DISABLE();
  __GPIOA_CLK_DISABLE();
  __GPIOB_CLK_DISABLE();
  __GPIOD_CLK_DISABLE();
}

/**
* Resets all of the 7-segment display pins, to shut the display off.
*/
void enableAllGPIOClocks(){
  __GPIOE_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
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

int toggleDigit(){
	currentDigit=(currentDigit+1)%4;
	return currentDigit;
}

void set_flag_display(int flag_index, int status){
	osSemaphoreWait(flag_sem_id,osWaitForever);
	if(flag_index == 0){
		reset_flag_global = status;
	}else if(flag_index == 1){
		sleep_flag_global = status;
	}else if(flag_index == 2){
		operation_flag_global = status;
	}
	osSemaphoreRelease(flag_sem_id);
}

int get_flag_display(int flag_index){
	int flag_status;
	osSemaphoreWait(flag_sem_id,osWaitForever);
	if(flag_index == 0){
		flag_status = reset_flag_global;
	}else if(flag_index == 1){
		flag_status = sleep_flag_global;
	}else if(flag_index == 2){
		flag_status = operation_flag_global;
	}
	osSemaphoreRelease(flag_sem_id);
	return flag_status;
}

void set_acc_pitch_and_roll(float *pitchRoll){
	osSemaphoreWait(acc_ready_sem_id,osWaitForever);
	pitch_roll_value_global[0] = pitchRoll[0];
	pitch_roll_value_global[1] = pitchRoll[1];
	osSemaphoreRelease(acc_ready_sem_id);
}

void get_acc_pitch_and_roll(float *pitchRoll){
	osSemaphoreWait(acc_ready_sem_id,osWaitForever);
	pitchRoll[0] = pitch_roll_value_global[0];
	pitchRoll[1] = pitch_roll_value_global[1];
	//temp_value[2] = acc_value_global[2];
	//printf("X: %f, Y: %f, Z: %f", temp_value[0], temp_value[1], temp_value[2]);
	osSemaphoreRelease(acc_ready_sem_id);
}

void set_key_pressed_display(char c){
	osSemaphoreWait(key_pressed_sem_id,osWaitForever);
	key_pressed_global = c;
	osSemaphoreRelease(key_pressed_sem_id);
}

char get_key_pressed_display(void){
	char temp;
	int val = osSemaphoreWait(key_pressed_sem_id,1);
	if (val > 0) {
      temp = key_pressed_global;
			osSemaphoreRelease(key_pressed_sem_id);
  }
	
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
	float acc_pitch_roll[2];
	
	char key_pressed;
	userInputState=1;
	enterRollState=1; 
	operatingModeRollMonitoring=1;
	viewAccelVals=1;

	user_pwm_set_led_brightness(500,0,0,500);//On startup, we are in enter Roll state
	int sleep_flag;
	int reset_flag;
	int operation_flag;
	while(1){
		
		osSignalWait(SIGNAL_READY, osWaitForever);
		key_pressed = get_key_pressed_display();
		sleep_flag = get_flag_display(sleep_flag_const);
		reset_flag = get_flag_display(reset_flag_const);
		operation_flag = get_flag_display(operation_flag_const);
		set_key_pressed_display('\0');
		/*if(key_pressed != '\0'){
			printf("Key is %c \n", key_pressed);
		}*/
		if (!sleepmode){
			HAL_TIM_Base_Start_IT(&htim2);
			digitSelect(&digitArray[0],toggleDigit());
			if (userInputState){
				if (entered_char_pointer==4){//nothing has been entered yet
					initializeDisplayToZero(digitArray);//display zero if nothing has been entered
				}
				if(key_pressed != '\0'&&key_pressed != '*'&&key_pressed != '#'){ //a number was entered
					//printf("Key Pressed is %c \n", key_pressed);
					enterNumberIntoBuffer(digitArray, key_pressed - '0');
				} else if(key_pressed == '#') {
					int concatedint = concatenateArray(digitArray);// concatenate array contents into one integer/.
					//printf("Input is %d \n", concatedint);
					if (enterRollState){ 
						inputRollExpected = concatedint;
						if(!reinit){//If not reinitializing, go to enterPitchState
							enterRollState=0;
							entered_char_pointer=4;
							initializeDisplayToZero(digitArray);
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
					//printf("Key Pressed is %c \n", key_pressed);
					deleteLastInBuffer(digitArray);
				}
			}else{ // Is in operatingMode
			//Update the value to be shown in 7-segment display.
				get_acc_pitch_and_roll(&acc_pitch_roll[0]);
				int retval = adjustBrightnessBasedOnACC(!operatingModeRollMonitoring,inputPitchExpected, inputRollExpected, &acc_pitch_roll[0]);
			
				if (viewAccelVals){
					toDisplay= retval;
				}else {
					if (operatingModeRollMonitoring)
						toDisplay= inputRollExpected;
					else 
						toDisplay= inputPitchExpected;
				}
				
				if(key_pressed == '1'){ //go to roll
					//printf("Key Pressed is %c \n", key_pressed);
					operatingModeRollMonitoring=1;
				} else if(key_pressed == '2'){ //go to pitch
					//printf("Key Pressed is %c \n", key_pressed);
					operatingModeRollMonitoring=0;
				} else if (key_pressed=='#'){
						//if in view input go to view accel, and vice versa.
						if (viewAccelVals)viewAccelVals=0;
						else viewAccelVals=1;
				}
				
				if (sleep_flag){//enter sleep mode
					//sleep_flag=0;
					set_flag_display(sleep_flag_const, 0);
					sleepmode=1;
					resetAll();
					disableAllGPIOClocks();
					stop_acc_thread();
					//HAL_TIM_Base_Stop_IT(&htim2);
					for (int i=0; i<4; i++)digitArray[i] = -1;
					user_pwm_set_led_brightness(0,0,0,0);//Dim the LEDS
				} else if (reset_flag){ //go to input mode for the relevant roll state
					//reset_flag=0;
					set_flag_display(reset_flag_const, 0);
					userInputState=1;
					reinit = 1;
					if (operatingModeRollMonitoring){
						enterRollState=1;
						entered_char_pointer=4;
						initializeDisplayToZero(digitArray);
						user_pwm_set_led_brightness(500,0,0,500);
					} else{
						enterRollState=0;
						entered_char_pointer=4;
						initializeDisplayToZero(digitArray);
						user_pwm_set_led_brightness(0,500,500,0);
					}
				}
				if (displayCounter==DISPLAY_COUNTER_MAX-1){ //Waiting for counter to reach 99 ensures display is updated less frequently than interrupt rate from timer (so as changes to be easily visible)
					intToArray(&digitArray[0],toDisplay);
				}
			} 
		} else{ //Is in sleep mode		  
			if(operation_flag){
				sleepmode=0;
				userInputState=0;
				set_flag_display(operation_flag_const, 0);
				enableAllGPIOClocks();
				start_acc_thread();
			}
		}
		displayCounter = (displayCounter+1)%DISPLAY_COUNTER_MAX;
		osSignalSet(Display_Thread_ID, SIGNAL_WAIT);
}
}
void initializeDisplayToZero(int * digitArray){
	int i;
	for (i=0; i<3; i++) digitArray[i] = -1;
	digitArray[3] = 0; //display zero if nothing has been entered
}

int concatenateArray(int * digitArray){
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

void enterNumberIntoBuffer(int * digitArray, int numberEntered){
	if (entered_char_pointer>1){ //as long as three digits haven't already been filled, a user can type a new letter
		for (int i=entered_char_pointer; i<4; i++){
			digitArray[i-1] =digitArray[i]; //shift what was already there
		}
		digitArray[3] = numberEntered; //add the pressed key to the last element.
		entered_char_pointer--;
	}
}

void deleteLastInBuffer(int * digitArray){
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

int adjustBrightnessBasedOnACC(int isPitch, float inputPitchExpected, float inputRollExpected, float* valsFromAcc){
	uint16_t diffMagnitudeForBrightnessPitch,diffMagnitudeForBrightnessRoll;//, diffMagnitudeForBrightnessRoll;
	float convertedPitchTo180scale,convertedRollTo180scale,  pitchDiff, rollDiff;
	float calculatedPitch = valsFromAcc[0]; //doing absolute values for now.
	float calculatedRoll = valsFromAcc[1];
		
	convertedPitchTo180scale= calculatedPitch+90.5;//adding .5 to cast to int properly
	convertedRollTo180scale= calculatedRoll+90.5;//adding .5 to cast to int properly
	// The diffMagnitudeForBrightness will be from 0 to 90 (because of abs). 
	pitchDiff = inputPitchExpected- convertedPitchTo180scale;
	diffMagnitudeForBrightnessPitch= (uint16_t) fabs( inputPitchExpected - convertedPitchTo180scale);
	// The diffMagnitudeForBrightness will be from 0 to 90 (because of abs).
	rollDiff = inputRollExpected- convertedRollTo180scale;
	diffMagnitudeForBrightnessRoll=  (uint16_t)fabs( inputRollExpected - convertedRollTo180scale);
		
	if (pitchDiff>0)
		user_pwm_set_led_brightness(EXCEEDS_MAX,0,diffMagnitudeForBrightnessPitch * 5.555555,EXCEEDS_MAX); //5.5555 = 500/90
	else
		user_pwm_set_led_brightness(EXCEEDS_MAX,diffMagnitudeForBrightnessPitch * 5.555555,0,EXCEEDS_MAX); //5.5555 = 500/90
	if (rollDiff>0)
		user_pwm_set_led_brightness(0,EXCEEDS_MAX,EXCEEDS_MAX,diffMagnitudeForBrightnessRoll * 5.555555); //5.5555 = 500/90
	else
		user_pwm_set_led_brightness(diffMagnitudeForBrightnessRoll * 5.555555,EXCEEDS_MAX,EXCEEDS_MAX,0); //5.5555 = 500/90

	if (isPitch){
			return (int)convertedPitchTo180scale;
	}else{
			return (int)convertedRollTo180scale;
	}
}

void user_pwm_set_led_brightness(uint16_t ld3, uint16_t ld4,uint16_t ld5,uint16_t ld6){
	TIM_OC_InitTypeDef sConfigOC;
  
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	 
	if(ld4!=EXCEEDS_MAX){
		sConfigOC.Pulse = ld4;
		HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
	} 
	
	if(ld3!=EXCEEDS_MAX){
		sConfigOC.Pulse = ld3;
		HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2);
	}
	if(ld5!=EXCEEDS_MAX){
		sConfigOC.Pulse = ld5;
		HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3);
	}
	if(ld6!=EXCEEDS_MAX){
		sConfigOC.Pulse = ld6;
		HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);
	}

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
	flag_sem_id = osSemaphoreCreate(osSemaphore(flag_sem), 1);
}