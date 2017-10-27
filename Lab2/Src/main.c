/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
#include "main.h"
#include "math.h"
#include "stm32f4xx_hal.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
int displayCounter=0;
int currentDigit=0;
int windowSizePassed=0;
int overflowed = 0; 
int spiked = 0; 


int overflowCounter = 0;
int spikeCounter = 0;

int flag;
int waveFlag;

int digitArray[4]={0,0,0,0} ;

float total = 0;
float voltage_reading = 0;
const float WINDOW = 1000.0;
const int WINDOW_INT=(int)WINDOW;
int window_counter = WINDOW_INT;
int counter = 0;
float buffer[WINDOW_INT];
float val_a = 0;
float val_b = 0;
const float MAX_ADC_BITS = 255.0f; 
const float MAX_VOLTS = 3.0f;  
const float VOLTS_PER_BIT = (MAX_VOLTS / MAX_ADC_BITS);
const	float MAX_ALLOWABLE = 3.402823466e+38F;

const float DELTA_THRESHOLD = (MAX_VOLTS*MAX_VOLTS*0.2*0.2); //Delta threshold set to 0.7 of full range.

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/**
* 	Lights up LEDs to indicate that the window size has not fully been collected yet.
*	  ORANGE LED ON, ALL ELSE OFF
*/
void preWindowState(){ 
		windowSizePassed=0;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
* 	Lights up LEDs to indicate that data is being collected as expected.
*	  GREEN LED ON,  ALL ELSE OFF
*/
void stableState(){
		windowSizePassed=1;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
* 	Lights up LEDs to indicate that an overflow has been detected.
*	  RED LED ON,  ALL ELSE OFF
*/
void overFlowState(){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
* 	Lights up LEDs to indicate that a data spike or unexpected value has been detected
*	  BLUE LED ON,  ALL ELSE OFF
*/
void spikeState(){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
}
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
	//HAL_NVIC_EnableIRQ(ADC_IRQn);
  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
	MX_TIM3_Init();
  MX_ADC2_Init();

  /* USER CODE BEGIN 2 */
	
	// Step(1): Start the Timer as interrupt
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	// Step(2): Start the ADC
	HAL_ADC_Start(&hadc2);
  /* USER CODE END 2 */
	preWindowState(); //Starts off at state before window size has been filled up
	floatTo4DigitArray(0);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);

	int counterTemp = 0;
  while (1)
  {
		//Update the value to be shown in 7-segment display.
		if (displayCounter==149){ //Waiting for counter to reach 99 ensures display is updated less frequently than interrupt rate from timer (so as changes to be easily visible)
			floatTo4DigitArray(voltage_reading);
		}
	  
		//Calculate RMS, and toggle which digit of 7-segment display to display
		if (flag==1){
			digitSelect(toggleDigit());
			getVoltage();
    }
		
		//Toggle square wave 
		if (waveFlag==1){
			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
			waveFlag=0;
    }
  }
  /* USER CODE END 3 */
}

/**
* Voltmeter functionality; Takes the RMS over the window size worth of samples.
*/
void getVoltage(){
		uint32_t adcVal = HAL_ADC_GetValue(&hadc2);
		float voltage = (float)adcVal * VOLTS_PER_BIT;
		float voltage_sqrt = voltage * voltage;
		if(window_counter>0){
			window_counter -= 1;
			val_a = val_a + voltage_sqrt;
			buffer[counter] = voltage_sqrt;
		}else{
			if (!windowSizePassed)
				stableState(); 
			if (spiked==1){
				spikeCounter= (spikeCounter+1)%500;
				if (spikeCounter == 499){
					stableState();
					spiked=0;
				}
			}
			val_b = val_b+buffer[counter];
			buffer[counter] = voltage_sqrt;
			val_a = val_a + voltage_sqrt;
			voltage_reading= (val_a-val_b)/WINDOW;
			voltage_reading = sqrt(voltage_reading);
			if (checkForSpike( buffer[counter],  buffer[counter-1], buffer[counter-2])){ //if spike is detected, overwrite it
				//buffer[counter]=buffer[counter-3];
				//buffer[counter-1]=buffer[counter-3];
				//buffer[counter-2]=buffer[counter-3];
			}
		}
		flag=0;
		checkForOverflow(voltage_reading);
		counter= (counter+1)%WINDOW_INT;
}

/**
* Checks for spikes in calculated values - if three successive values have a threshold level of difference (for example, 1,3,1) between them, we call this a spike.
*/
int checkForSpike(float a, float b, float c){
	if(spiked==0&&fabs(a-b)>DELTA_THRESHOLD &&fabs(b-c)>DELTA_THRESHOLD){//spike detected.
		spikeState();
		spiked=1;
		return 1;
	}
	return 0;
}

/**
* When we get near the overflow value, we reset the accumulators.
*/
void checkForOverflow(float voltage_reading){
	float max= MAX_ALLOWABLE/2;
	if (val_a> max){
		val_b= 0;
		val_a= 0;
		window_counter= WINDOW_INT;
		if (overflowed==0){
			overFlowState();
			overflowed=1;
		}
	} else {
		if (overflowed==1){
			overflowCounter = (overflowCounter+1)%200; 
			if (overflowCounter==199){
				stableState();
				overflowed=0;
			}
		}
	}
}


/**
* Takes the first  to a 4 digit array
*/
void floatTo4DigitArray(float fVal)  
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
* Sets the segments pins so that the appropriate segments are lit according to number
* A - 7
* B - 8
* C - 9
* D - 10 
* E - 11
* F - 12
* G - 13
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

/**
* Selects which of the four digits to enable
*/
int toggleDigit(){
	currentDigit=(currentDigit+1)%4;
	return currentDigit;
}

/**
* Depending on which bit is selected, enables the digit in 
*/
void digitSelect(int currentDigit ){
		if (currentDigit==0){ 
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_SET); //also set the decimal point always for first digit
		}
		if (currentDigit==1){ 
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_RESET);

		}if (currentDigit==2){ 
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_RESET);

		}if (currentDigit==3){ 
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_14,GPIO_PIN_RESET);
		}
		displayInt(digitArray[currentDigit]);
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
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