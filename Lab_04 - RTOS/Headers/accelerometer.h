void Calibrate_ACC_Value(void);
void Read_Raw_ACC(void const *argument);
void start_acc_thread(void);
void Get_Final_ACC_Value(float *value);
void start_acc_thread(void);
void Get_Final_Pitch_And_Roll(float *value, float *pitchRoll);
float calculatePitchAngleFromAccVals(float ax, float ay, float az);
float calculateRollAngleFromAccVals(float ax, float ay, float az);
