/*
 * Integration/main.ino
 *
 * 모든 모듈을 통합 제어하는 메인 스케치입니다.
 * 각 모듈의 헤더 파일은 Modules 폴더에 위치하며 상대 경로로 참조합니다.
 */

// ==========================================
// [Source: Header Includes]
// ==========================================
#include "../Modules/ArduCAM-OV2640/Camera_module.h" // [Source: ArduCAM]
#include "../Modules/DCMotor/DCMotor.h"              // [Source: DCMotor]
#include "../Modules/DS18B20-/Temp_module.h"         // [Source: DS18B20]
#include "../Modules/EC-sensor/EC_module.h"          // [Source: EC-sensor]

void setup() {
  // [Source: Common] 공통 통신 설정
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n===== [Integration System Start] =====");

  // ----------------------------------------
  // [Source: EC-sensor] 초기화
  // ----------------------------------------
  Serial.println(">> Initializing EC Sensor...");
  ec_init();

  // ----------------------------------------
  // [Source: DS18B20] 온도 센서 초기화
  // ----------------------------------------
  Serial.println(">> Initializing Temp Sensor...");
  temp_init();

  // ----------------------------------------
  // [Source: DCMotor] 모터 초기화
  // ----------------------------------------
  Serial.println(">> Initializing DC Motor...");
  // 원래 project.ino의 setup에 있던 내용
  ledcAttach(M1A_PIN, 1000, 8);
  ledcAttach(M1B_PIN, 1000, 8);
  stopMotor();

  // ----------------------------------------
  // [Source: ArduCAM] 카메라 초기화
  // ----------------------------------------
  Serial.println(">> Initializing Camera...");
  camera_init();

  Serial.println("===== [All Systems Ready] =====\n");
}

void loop() {
  // ====================================================
  // [Source: DCMotor] 모터 제어 명령 처리부 (원래 project.ino loop)
  // ====================================================
  if (Serial.available() > 0) {
    // 주의: 다른 모듈(EC, Camera)도 시리얼 입력을 사용할 수 있으므로
    // 여기서 모든 시리얼 입력을 처리하거나, 명령어 형식을 구분해야 합니다.
    // 현재는 DCMotor의 명령어("INSPECT", "STOP")를 우선 처리합니다.
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "INSPECT" && currentState == IDLE) {
      startInspection();
    } else if (cmd == "STOP") {
      stopInspection();
    }
  }

  updateInspection(); // 모터 상태 기계 업데이트

  static unsigned long lastMotorStatusTime = 0;
  if (millis() - lastMotorStatusTime >= 500) {
    sendStatus(); // 상태 보고 JSON 전송
    lastMotorStatusTime = millis();
  }

  // ====================================================
  // [Source: EC-sensor] 수질 측정 루프 (원래 EC_module.ino loop)
  // ====================================================
  ec_loop();

  // ====================================================
  // [Source: DS18B20] 온도 측정 (원래 temp.ino loop)
  // ====================================================
  static unsigned long lastTempTime = 0;
  if (millis() - lastTempTime >= 1000) { // 1초마다 실행 (block 방지)
    float currentTemp = temp_read();
    Serial.print("[Temp] ");
    Serial.print(currentTemp);
    Serial.println(" C");
    lastTempTime = millis();
  }

  // ====================================================
  // [Source: ArduCAM] 카메라 스트리밍 (원래 camera.ino loop)
  // ====================================================
  // 주의: 카메라 캡처는 시간이 오래 걸릴 수 있어(Blocking),
  // 모터 제어 등에 영향을 줄 수 있습니다. 필요 시 타이머로 호출 빈도를
  // 조절하세요.
  camera_loop();

  // ====================================================
  // [Source: Common] 시스템 안정화 딜레이
  // ====================================================
  delay(10); // 너무 빠른 루프 방지 (ArduCAM/Motor 반응성 고려하여 작게 설정)
}