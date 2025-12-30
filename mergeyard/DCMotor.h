#ifndef DCMotor_H
#define DCMotor_H

#include <Arduino.h>

// 핀 설정
#define M1A_PIN 26 // 모터 드라이버 M1A (정방향)
#define M1B_PIN 25 // 모터 드라이버 M1B (역방향)

// 상태 열거형 정의 (헤더에 있어야 메인과 CPP 모두 알 수 있음)
enum State { IDLE, FORWARD, STOP_PHASE, REVERSE };

// 변수 선언 (외부에서 접근 가능하도록 extern 사용)
extern State currentState;
extern unsigned long stateStartTime;
extern int motorSpeed;

// 함수 선언
void startInspection();
void stopInspection();
void updateInspection();
void sendStatus();

// 내부 제어 함수지만 project.ino setup()에서 사용하므로 공개
void stopMotor();
void forwardMotor();
void reverseMotor();

#endif