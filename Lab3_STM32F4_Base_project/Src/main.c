/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "lis3dsh.h"
#include "keypad.h"
#include "accelerometer.h"
#include "tim.h"
#include "math.h"
#include "display.h"

#define PI 3.14159265

void user_pwm_setvalue(uint16_t value);
void user_pwm_set_led_brightness(uint16_t ld3, uint16_t ld4,uint16_t ld5,uint16_t ld6);
void adjustBrightnessBasedOnACC(int isPitch, float expectedPitchOrRoll, float* valsFromAcc);
int toggleDigit();
void enterNumberIntoBuffer(int numberEntered);
void deleteLastInBuffer();
void initializeDisplayToZero();
int concatenateArray();

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void initializeACC			(void);
void MX_NVIC_Init(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
int concatenate(int x, int y);
void toggleflash(int);

int SysTickCount;
int acc_flag;
int reset_flag;
int sleep_flag;
int wakeup_flag;
int digselect_flag;

int counter = 0;
int dataReady = 0;
int timer4counter;
int pwm_value,step;
int testtoggle=0;
int currentDigit=0;
int displayCounter=0;
int digitArray[4]={0,0,0,0} ;
const int DISPLAY_COUNTER_MAX = 500; 
int toDisplay=0;

//	const int reset_threshold = 100000;
//	const int sleep_threshold = 350000;

//-- fsm
int userInputState; //if not in userInputState, is in operatingMode
int enterRollState; // if not in enterRollState, is in enterPitchState
int operatingModeRollMonitoring; // if not operatingModeRollMonitoring, is operatingModePitchMonitoring
int sleepmode=0;
int inputRollExpected = 0;
int inputPitchExpected = 0;

int rollstatecounter=0;
int flashflag=0;
int flashduration=10000;

int entered_char_buffer[4]={-1,-1,-1,0};
int entered_char_pointer=4; //index of the last element (one further than the last because no input at first)

//--
char prevkeypressed;
int pressedCounter=0;
int loopCounter=0;
int main(void)
{
	timer4counter=0;
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
	initializeACC	();
	MX_NVIC_Init();
	MX_TIM4_Init();
	MX_TIM2_Init();
	
  HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start(&htim4);
	
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
	
  userInputState=1;
	enterRollState=1; //On startup, we are in enter Roll state
	operatingModeRollMonitoring=1; //the first operating mode we enter will be roll monitoring
	user_pwm_set_led_brightness(500,0,0,500);//On startup, we are in enter Roll state

	//HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);//or1
	//int counter = 0;
  while (1)
  {
		//in main while loop display counter goes on as long as its not in sleep mode
		if (!sleepmode){
		//	if (displayCounter==DISPLAY_COUNTER_MAX-1) //Waiting for counter to reach 99 ensures display is updated less frequently than interrupt rate from timer (so as changes to be easily visible)
		//			intToArray(&digitArray[0],toDisplay);
			if(digselect_flag==1)
					digitSelect(&digitArray[0],toggleDigit());
			
			if (userInputState){
				if (entered_char_pointer==4){//nothing has been entered yet
						initializeDisplayToZero();//display zero if nothing has been entered
				}
				char key_pressed = Read_KP_Value();
				if(key_pressed != '\0'&&key_pressed != '*'&&key_pressed != '#'){ //a number was entered
					printf("Key Pressed is %c \n", key_pressed);
					enterNumberIntoBuffer(key_pressed - '0');
				} else if(key_pressed == '#') {
					//for now just go directly into roll monitoring. later IF VALUE IS GREATER THAN 180, CAP at 180 DIsplay 180 before entering.
					int concatedint = concatenateArray();// concatenate array contents into one integer/.
					printf("Input is %d \n", concatedint);
					if (enterRollState){ //go to enterPitchState
						inputRollExpected = concatedint;
						//enterPitchState
						enterRollState=0;
						entered_char_pointer=4;
						initializeDisplayToZero();
						user_pwm_set_led_brightness(0,500,500,0);//On startup, we enter Pitch state
					}	else{ //otherwise is enterPitchState, go to roll monitoring
						inputPitchExpected = concatedint;
						userInputState=0;
						enterRollState=1;
					}
				} else if (key_pressed=='*'){
					printf("Key Pressed is %c \n", key_pressed);
					deleteLastInBuffer();
				}
			}else{ // if not userInputState, is in operatingMode
			//Update the value to be shown in 7-segment display.
				if (displayCounter==DISPLAY_COUNTER_MAX-1) //Waiting for counter to reach 99 ensures display is updated less frequently than interrupt rate from timer (so as changes to be easily visible)
					intToArray(&digitArray[0],toDisplay);
				
				if(acc_flag == 1){
					float acc_value[3]= {99,99,99};
					Calibrate_ACC_Value(&acc_value[0]);
					//printf("Temp: X: %3f   Y: %3f   Z: %3f \n",acc_value[0], acc_value[1], acc_value[2]);
					if (operatingModeRollMonitoring){
						adjustBrightnessBasedOnACC(0, inputRollExpected, &acc_value[0]);
					} else{//if not in operatingModeRollMonitoring is operatingModePitchMonitoring
						adjustBrightnessBasedOnACC(1, inputPitchExpected, &acc_value[0]);
					}
					acc_flag = 0;
				}
				char key_pressed = Read_KP_Value();
				if(key_pressed == '1'){ //go to roll
					printf("Key Pressed is %c \n", key_pressed);
					prevkeypressed=key_pressed;
					operatingModeRollMonitoring=1;
				} else if(key_pressed == '2'){ //go to pitch
					printf("Key Pressed is %c \n", key_pressed);
					prevkeypressed=key_pressed;
					operatingModeRollMonitoring=0;
				}
			} 
		}
		
		//loopCounter++;
		displayCounter = (displayCounter+1)%DISPLAY_COUNTER_MAX;
	}
}
void initializeDisplayToZero(){
		for (int i=0; i<3; i++)
			digitArray[i] = -1;
		digitArray[3] = 0; //display zero if nothing has been entered
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

int concatenateArray(){
		if (entered_char_pointer==4) entered_char_pointer=3;//if nothing was entered just take the 0
		int concatedint = digitArray[entered_char_pointer];
		for (int i = entered_char_pointer+1; i<4; i++){
			concatedint = concatenate(concatedint,digitArray[i] );
		}
		return concatedint;
}

int concatenate(int x, int y) {
    int pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

int toggleDigit(){
	currentDigit=(currentDigit+1)%4;
	return currentDigit;
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

void adjustBrightnessBasedOnACC(int isPitch, float expectedPitchOrRoll, float* valsFromAcc){
	uint16_t diffMagnitudeForBrightness;
	float calculated, convertedTo180scale, diff;
	float ax = valsFromAcc[0]; //doing absolute values for now.
	float ay = valsFromAcc[1];
	float az = valsFromAcc[2];
	if (isPitch){
		calculated = calculatePitchAngleFromAccVals(ax, ay, az);
		convertedTo180scale= calculated+90;
		// The diffMagnitudeForBrightness will be from 0 to 90 (because of abs). 
		diff = expectedPitchOrRoll- convertedTo180scale;
		diffMagnitudeForBrightness= (uint16_t) fabs( expectedPitchOrRoll - convertedTo180scale);
	
		if (diff>0)
			user_pwm_set_led_brightness(0,0,diffMagnitudeForBrightness * 5.555555,0); //5.5555 = 500/90
		else
			user_pwm_set_led_brightness(0,diffMagnitudeForBrightness * 5.5555550,0,0); //5.5555 = 500/90
	}
	else {
		calculated = calculateRollAngleFromAccVals(ax, ay, az);
		convertedTo180scale= calculated+90;
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

/**
* led brightness can range from 0 to 1000
* Brightness =1000 is 180 degrees off
* Brightness = 500 is 90 degrees off
* Brightness = 0 is 0 degrees off
* PD12(LD4)- green, PD13(LD3)- orange, PD14(LD5)-red, PD15(LD6)-blue
* Only LD3,LD6 OR LD4,LD5 will be on at the same time, depending on whether pitch was selected or roll.
*/
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

/** System Clock Configuration
	The clock source is configured as external at 168 MHz HCLK
*/

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
	
  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */
void initializeACC(void){
	LIS3DSH_InitTypeDef 		Acc_instance;
	/* Private variables ---------------------------------------------------------*/
	LIS3DSH_DRYInterruptConfigTypeDef configDef;
	SPI_HandleTypeDef SPI_Handle;
	
	Acc_instance.Axes_Enable				= LIS3DSH_XYZ_ENABLE;
	Acc_instance.AA_Filter_BW				= LIS3DSH_AA_BW_50;
	Acc_instance.Full_Scale					= LIS3DSH_FULLSCALE_2;
	Acc_instance.Power_Mode_Output_DataRate		= LIS3DSH_DATARATE_50;
	Acc_instance.Self_Test					= LIS3DSH_SELFTEST_NORMAL;
	Acc_instance.Continous_Update   = LIS3DSH_ContinousUpdate_Enabled;
	
	configDef.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;
	configDef.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;
	configDef.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;
	LIS3DSH_Init(&Acc_instance);	
	
	LIS3DSH_DataReadyInterruptConfig(&configDef);
	HAL_SPI_MspInit(&SPI_Handle);
	/* Enabling interrupt conflicts with push button. Be careful when you plan to 
	use the interrupt of the accelerometer sensor connceted to PIN A.0

	*/
}

void MX_NVIC_Init(void){
  /* Enable and set EXTI Line0 Interrupt */
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  
}

void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}




#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
