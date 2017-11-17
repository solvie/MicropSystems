void displayInt(int toDisplay);
void intToArray(int * array, int number);
void resetAll(void);
void digitSelect(int* digitArray,int currentDigit );
void Display_Thread(void const *argument);
void start_display_thread(void);
void set_key_pressed_display(char c);
char get_key_pressed_display(void);
void set_acc_value_display(float *temp_value);
void set_acc_pitch_and_roll(float *pitchRoll);
void get_acc_value_display(float *temp_value);
void get_acc_pitch_and_roll(float *pitchRoll);
void initializeDisplayToZero(int* array);
int concatenateArray(int* array);
int concatenate(int x, int y);
void enterNumberIntoBuffer(int* array, int numberEntered);
void deleteLastInBuffer(int* array);
void adjustBrightnessBasedOnACC(int isPitch, float expectedPitch, float expectedRoll, float* valsFromAcc);
void user_pwm_set_led_brightness(uint16_t ld3, uint16_t ld4,uint16_t ld5,uint16_t ld6);
void set_flag_display(int flag_index, int status);
//float calculatePitchAngleFromAccVals(float ax, float ay, float az);
//float calculateRollAngleFromAccVals(float ax, float ay, float az);