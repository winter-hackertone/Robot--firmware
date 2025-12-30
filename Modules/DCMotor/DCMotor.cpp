#include "DCMotor.h"

// 변수 정의 (실체 생성)
State currentState = IDLE;
unsigned long stateStartTime = 0;
int motorSpeed = 200;

void startInspection() {
  currentState = FORWARD;
  stateStartTime = millis();
  forwardMotor();
  Serial.println("{\"type\":\"response\",\"status\":\"started\"}");
}

void stopInspection() {
  currentState = IDLE;
  stopMotor();
  Serial.println("{\"type\":\"response\",\"status\":\"stopped\"}");
}

void updateInspection() {
  if (currentState == IDLE)
    return;

  unsigned long elapsed = millis() - stateStartTime;

  switch (currentState) {
  case FORWARD:
    if (elapsed >= 5000) {
      currentState = STOP_PHASE;
      stateStartTime = millis();
      stopMotor();
    }
    break;

  case STOP_PHASE:
    if (elapsed >= 3000) {
      currentState = REVERSE;
      stateStartTime = millis();
      reverseMotor();
    }
    break;

  case REVERSE:
    if (elapsed >= 5000) {
      currentState = IDLE;
      stopMotor();
    }
    break;
  }
}

void sendStatus() {
  String state;
  unsigned long elapsed = 0;

  switch (currentState) {
  case IDLE:
    state = "idle";
    break;
  case FORWARD:
    state = "forward";
    elapsed = millis() - stateStartTime;
    break;
  case STOP_PHASE:
    state = "paused";
    elapsed = millis() - stateStartTime;
    break;
  case REVERSE:
    state = "reverse";
    elapsed = millis() - stateStartTime;
    break;
  }

  Serial.print("{\"type\":\"status\",\"state\":\"");
  Serial.print(state);
  Serial.print("\",\"elapsed\":");
  Serial.print(elapsed);
  Serial.println("}");
}

void stopMotor() {
  ledcWrite(M1A_PIN, 0);
  ledcWrite(M1B_PIN, 0);
}

void forwardMotor() {
  ledcWrite(M1A_PIN, motorSpeed);
  ledcWrite(M1B_PIN, 0);
}

void reverseMotor() {
  ledcWrite(M1A_PIN, 0);
  ledcWrite(M1B_PIN, motorSpeed);
}