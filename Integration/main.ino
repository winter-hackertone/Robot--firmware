#include "../Modules/ArduCAM-OV2640/Camera_module.h" // [Source: ArduCAM]
#include "../Modules/DCMotor/DCMotor.h"              // [Source: DCMotor]
#include "../Modules/DS18B20-/Temp_module.h"         // [Source: DS18B20]
#include "../Modules/EC-sensor/EC_module.h"          // [Source: EC-sensor]
#include "../Modules/Network/Network_module.h"       // [Source: Network]

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
  camera_init();

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

    // 카메라 프레임 캡처
    uint8_t *imgBuf = NULL;
    size_t imgLen = 0;

    Serial.println(">> Capturing frame for streaming...");
    if (capture_frame(&imgBuf, &imgLen)) {
      // 캡처 성공 시 통합 데이터 업로드
      upload_integrated_data(currentTemp, currentEC, imgBuf, imgLen);
    } else {
      // 캡처 실패 시 데이터만이라도 업로드 (이미지 없이)
      upload_integrated_data(currentTemp, currentEC, NULL, 0);
    }

    lastUploadTime = millis();
  }

  delay(10); // 시스템 안정화
}