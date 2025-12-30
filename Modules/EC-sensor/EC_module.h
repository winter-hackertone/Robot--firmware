#ifndef EC_MODULE_H
#define EC_MODULE_H

#include <Arduino.h>

#define SIZE 5
#define MIN_AVG_THRESHOLD 50
#define WARNING_MULTIPLIER 1.5
#define TDS_PIN 32
#define BUZZER_PIN 4

extern int readings[SIZE];
extern int readIndex;
extern long total;
extern float averageADC;
extern unsigned long measurementCount;
extern int measurementState;

// 초기화 및 루프 함수
void ec_init();
void ec_loop();

int read_sensor_adc();
void update_moving_average(int current_adc);
void check_and_alert(int current_adc);
void display_data(int current_adc);
void set_measurement_state(int new_state);

// 데이터 조회 함수
float get_average_ec();

#endif