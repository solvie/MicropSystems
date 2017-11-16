#include "gpio.h"
#include "keypad.h"
#include "cmsis_os.h"  
#include "display.h"
/*
connection: 
PB12,PB13,PB14,PB15,PD8,PD9,PD10,PD11

Row:
	RB0 -> PD8
	RB1 -> PD9
	RB2 -> PD10
	RB3 -> PD11

Col:
	RB4 -> PB12
	RB5 -> PB13
	RB6 -> PB14
	RB7 -> PB15 ---Disable
*/
osThreadId Read_KP_Value_Id;
osThreadDef(Read_KP_Value, osPriorityNormal, 1, 0);
extern int reset_flag;
extern int sleep_flag;
extern int operation_flag;
extern osSemaphoreId read_kp_flag_sem;    
extern char key_pressed;
const float KEYPAD_MAP [4][3] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};

/**
* Helper for Read_KP_Value.
* Set rows as output and columns as input; if no button is pressed, column line inputs
* will all be read as 1 due to pullup resistors; if button is pressed, that column will go 0.
*/
void Row_Out_Col_In(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	//row -> Output
	__GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	//col -> Input
 	__GPIOB_CLK_ENABLE();
  	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  	GPIO_InitStruct.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
* Helper for Read_KP_Value.
* Set rows as input and columns as output; if no button is pressed, row line inputs
* will all be read as 1 due to pullup resistors; if button is pressed, that row will be set 0.
*/
void Row_In_Col_Out(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	//row -> Input
	__GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	//col -> Output
 	__GPIOB_CLK_ENABLE();
  	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
* Helper for Read_KP_Value.
* Retrieves the index of the column where the key was pressed by finding which pin was reset.
*/
int Get_Col_Pin_In_Reset_Mode(void){
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET){
		return 0;
	}else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET){
		return 1;
	}else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET){
		return 2;
	}else{
		return -1;
	}
}

/**
* Helper for Read_KP_Value.
* Retrieves the index of the row where the key was pressed by finding which pin was reset.
*/
int Get_Row_Pin_In_Reset_Mode(void){
	if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_8) == GPIO_PIN_RESET){
		return 0;
	}else if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9) == GPIO_PIN_RESET){
		return 1;
	}else if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) == GPIO_PIN_RESET){
		return 2;
	}else if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) == GPIO_PIN_RESET){
		return 3;
	}else{
		return -1;
	}
}

/**
* Reads and returns the value pressed on the keypad, as well as setting flags to 
* announce detection of long presses on the * and # keys.
*/
void Read_KP_Value(void const *argument){
	int col_index;
	int row_index;
	int data_ok;
	int isSemReady;
	static int counter = 0;
	const int key_pressed_threshold = 10000;
	const int reset_threshold = 30000;
	const int sleep_threshold = 100000;
	static char temp = 0;

	while(1){
		//isSemReady = osSemaphoreWait(read_kp_flag_sem, 0);
		//if(isSemReady == 0){
		//	osThreadYield();
		//}
		data_ok = 0;
		counter = 0;
		temp = 0;
		while(data_ok == 0){
			Row_Out_Col_In();
			col_index = Get_Col_Pin_In_Reset_Mode();
			Row_In_Col_Out();
			row_index = Get_Row_Pin_In_Reset_Mode();
			//printf("Counter is %d\n", counter);
			if(row_index!=-1 && col_index!=-1){
				if(counter == 0){
					temp = KEYPAD_MAP[row_index][col_index];
					counter += 1;
				}else{
					if(temp == '*'){
						if(temp == KEYPAD_MAP[row_index][col_index]){
							counter ++;
							if(counter > sleep_threshold){
								sleep_flag = 1;
								temp = 0;
								counter = 0;
								data_ok = 1;
								key_pressed = '\0';
							}
						}
					}else if(temp=='#'){
						if(temp == KEYPAD_MAP[row_index][col_index]){
							counter ++;
							if(counter > sleep_threshold){
								operation_flag = 1;
								temp = 0;
								counter = 0;
								data_ok = 1;
								key_pressed = '\0';
							}
						}
					}else{
						if(counter > key_pressed_threshold){
							counter = 0;
							temp = 0;
							data_ok = 1;
							key_pressed = KEYPAD_MAP[row_index][col_index];
						}
						if(temp == KEYPAD_MAP[row_index][col_index]){
							counter ++;
						}else{
							counter = 0;
							temp = 0;
						}
					}
				}
			}else{
				if(temp == '*'){
						if(counter > reset_threshold){
								reset_flag = 1;
								counter = 0;
								temp = 0;
								key_pressed = '\0';
								data_ok = 1;
						}else if(counter > key_pressed_threshold && counter < reset_threshold){
								counter = 0;
								temp = 0;
								key_pressed = '*';
								data_ok = 1;
						}
							
				}else if(temp == '#'){
					if(counter > key_pressed_threshold){
							counter = 0;
							temp = 0;
							key_pressed = '#';
							data_ok = 1;
					}
							
				}else{
					temp = 0;
					counter = 0;
					key_pressed = '\0';
					data_ok = 1;
				}

				}
			}
		//osSemaphoreRelease(read_kp_flag_sem);
		if(key_pressed != '\0'){
			printf("Key is %c \n", key_pressed);
		}
		set_key_pressed_display(key_pressed);
		
		osDelay(10);  
	}
}

void start_kp_thread(void){
	Read_KP_Value_Id = osThreadCreate(osThread(Read_KP_Value), NULL); 
}