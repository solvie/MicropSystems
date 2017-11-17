/*******************************************************************************
  * @file    main.c
  * @author  Amirhossein Shahshahani
	* @version V1.2.0
  * @date    10-Nov-2017
  * @brief   This file demonstrates flasing one LED at an interval of one second
	*          RTX based using CMSIS-RTOS 
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "lis3dsh.h"
#include "gpio.h"
#include "accelerometer.h"
#include "keypad.h"
#include "tim.h"
#include "display.h"

#define PI 3.14159265

//The thread code is written in Thread_LED.c, just telling the toolchain that the 
//functions are declared externally
osSemaphoreId read_kp_flag_sem;                         // Semaphore ID
osSemaphoreDef(read_kp_flag_sem);                       // Semaphore definition
extern void initializeLED_IO			(void);
extern void start_Thread_LED			(void);
extern void Thread_LED(void const *argument);
extern osThreadId tid_Thread_LED;
void SystemClock_Config(void);
void initializeACC(void);
void MX_NVIC_Init(void);
char key_pressed;
int displayCounter;
const int DISPLAY_COUNTER_MAX = 10000; 
//extern osThreadId Read_KP_Value_Id;


/**
	These lines are mandatory to make CMSIS-RTOS RTX work with te new Cube HAL
*/
#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif

void thread_temp(void const *argument){
	int isSemReady;
	while(1){
		isSemReady = osSemaphoreWait(read_kp_flag_sem, osWaitForever);
		if(isSemReady == 0){
			osThreadYield();
		}
		//osThreadTerminate(Read_KP_Value_Id);
		if(key_pressed != '\0'){
			printf("char type is %c \n", key_pressed);
		}
		//start_kp_thread();
		osSemaphoreRelease(read_kp_flag_sem); 
	}
}
/**
  * Main function
  */

void start_temp_thread(void){
	osThreadId tid_thread1;  
	osThreadDef(thread_temp, osPriorityNormal, 1, 0);
	tid_thread1 = osThreadCreate(osThread(thread_temp), NULL);
}
 
int main (void) {

  osKernelInitialize();                     /* initialize CMSIS-RTOS          */

	read_kp_flag_sem = osSemaphoreCreate(osSemaphore(read_kp_flag_sem), 1);
	MX_GPIO_Init();
  HAL_Init();                               /* Initialize the HAL Library     */
	initializeACC();
	MX_NVIC_Init();
  SystemClock_Config();                     /* Configure the System Clock     */
	MX_TIM4_Init();
	MX_TIM2_Init();
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
	start_acc_thread();
	start_kp_thread();
	start_display_thread();
	//start_temp_thread();
	/* User codes goes here*/
  //initializeLED_IO();                       /* Initialize LED GPIO Buttons    */
  //start_Thread_LED();                       /* Create LED thread              */
	/* User codes ends here*/
  
	osKernelStart();                          /* start thread execution         */

}

/**
  * System Clock Configuration
  */

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}
void MX_NVIC_Init(void){
  /* Enable and set EXTI Line0 Interrupt */
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  
}

void initializeACC(void){
	LIS3DSH_InitTypeDef 		Acc_instance;
	/* Private variables ---------------------------------------------------------*/
	LIS3DSH_DRYInterruptConfigTypeDef configDef;
	SPI_HandleTypeDef SPI_Handle;
	
	Acc_instance.Axes_Enable				= LIS3DSH_XYZ_ENABLE;
	Acc_instance.AA_Filter_BW				= LIS3DSH_AA_BW_200;
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

void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}