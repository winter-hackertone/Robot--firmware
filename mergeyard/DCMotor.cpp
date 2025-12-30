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
      Serial.println("[Motor] FORWARD -> STOP_PHASE");
    }
    break;

  case STOP_PHASE:
    if (elapsed >= 3000) {
      currentState = REVERSE;
      stateStartTime = millis();
      reverseMotor();
      Serial.println("[Motor] STOP_PHASE -> REVERSE");
    }
    break;

  case REVERSE:
    if (elapsed >= 5000) {
      currentState = IDLE;
      stopMotor();
      Serial.println("[Motor] REVERSE -> IDLE (Complete)");
    }
    break;
  }
}

void sendStatus() {
  String stateStr;
  unsigned long elapsed = 0;

  // 수동 조작 감지 로직 (IDLE 상태일 때 핀 신호 확인)
  bool pinA = digitalRead(M1A_PIN);
  bool pinB = digitalRead(M1B_PIN);

  if (currentState == IDLE) {
    if (pinA && !pinB)
      stateStr = "manual_forward";
    else if (!pinA && pinB)
      stateStr = "manual_reverse";
    else if (pinA && pinB)
      stateStr = "manual_active";
    else
      stateStr = "idle";
  } else {
    switch (currentState) {
    case FORWARD:
      stateStr = "forward";
      break;
    case STOP_PHASE:
      stateStr = "paused";
      break;
    case REVERSE:
      stateStr = "reverse";
      break;
    default:
      stateStr = "unknown";
      break;
    }
    elapsed = millis() - stateStartTime;
  }

  Serial.print("{\"type\":\"status\",\"state\":\"");
  Serial.print(stateStr);
  Serial.print("\",\"elapsed\":");
  Serial.print(elapsed);
  Serial.println("}");
}

void stopMotor() {
  // PWM 중지 및 핀을 입력 모드로 변경하여 수동 신호 감지 허용
  ledcWrite(M1A_PIN, 0);
  ledcWrite(M1B_PIN, 0);
  pinMode(M1A_PIN, INPUT);
  pinMode(M1B_PIN, INPUT);
}

void forwardMotor() {
  // 핀을 다시 출력 모드로 변경 (ledcAttach가 필요할 수 있음)
  pinMode(M1A_PIN, OUTPUT);
  pinMode(M1B_PIN, OUTPUT);
  ledcAttach(M1A_PIN, 1000, 8);
  ledcAttach(M1B_PIN, 1000, 8);
  ledcWrite(M1A_PIN, motorSpeed);
  ledcWrite(M1B_PIN, 0);
}

void reverseMotor() {
  pinMode(M1A_PIN, OUTPUT);
  pinMode(M1B_PIN, OUTPUT);
  ledcAttach(M1A_PIN, 1000, 8);
  ledcAttach(M1B_PIN, 1000, 8);
  ledcWrite(M1A_PIN, 0);
  ledcWrite(M1B_PIN, motorSpeed);
}