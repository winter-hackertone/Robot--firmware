// #include "Camera_module.h"  // [Source: ArduCAM]
#include "DCMotor.h"        // [Source: DCMotor]
#include "EC_module.h"      // [Source: EC-sensor]
#include "Network_module.h" // [Source: Network]
#include "Temp_module.h"    // [Source: DS18B20]

// 타이머 변수
unsigned long lastUploadTime = 0;
const unsigned long uploadInterval =
    2000; // 2초마다 데이터 업로드 (스트리밍 포함)

void setup() {
  // [Source: Common] 공통 통신 설정
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n===== [Integration System Start] =====");
  Serial.println(">>> CHECKPOINT: MERGEYARD V1.2 <<<");

  // ----------------------------------------
  // [Source: Network] WiFi 및 네트워크 초기화
  // ----------------------------------------
  network_init();

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
  ledcAttach(M1A_PIN, 1000, 8);
  ledcAttach(M1B_PIN, 1000, 8);
  stopMotor();

  // ----------------------------------------
  // [Source: ArduCAM] 카메라 초기화
  // ----------------------------------------
  Serial.println(">> Initializing Camera...");
  // camera_init();

  Serial.println("===== [All Systems Ready] =====\n");
}

void loop() {
  // 1. 시리얼 명령 처리 (로컬 제어용)
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "INSPECT" && currentState == IDLE) {
      startInspection();
    } else if (cmd == "STOP") {
      stopInspection();
    } else if (cmd == "EC_ON") {
      set_measurement_state(1);
    } else if (cmd == "EC_OFF") {
      set_measurement_state(0);
    }
  }

  // 2. 모터 제어 명령 폴링 (서버 제어용)
  static unsigned long lastPollTime = 0;
  if (millis() - lastPollTime >= 3000) { // 3초마다 폴링
    poll_motor_command();
    lastPollTime = millis();
  }

  // 3. 각 모듈 상태 업데이트
  updateInspection();
  ec_loop();

  // 4. 데이터 통합 업로드 및 스트리밍
  if (millis() - lastUploadTime >= uploadInterval) {
    float currentTemp = temp_read();
    float currentEC = get_average_ec();

    Serial.print("Water Temp: ");
    Serial.print(currentTemp);
    Serial.println(" C");

    // 모터 상태 주기적 출력
    sendStatus();

    Serial.println("[Main] Calling upload_integrated_data...");
    // 데이터 통합 업로드 (이미지 없이)
    upload_integrated_data(currentTemp, currentEC, NULL, 0);

    lastUploadTime = millis();
  }

  delay(10); // 시스템 안정화

  // 디버그용 하트비트 (루프가 살아있는지 확인)
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 5000) {
    Serial.println("[System] Loop is running...");
    lastHeartbeat = millis();
  }
}