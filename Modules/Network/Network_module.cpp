#include "Network_module.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <base64.h> // ESP32 core base64 library

// DCMotor의 상태와 함수를 사용하기 위해 포함 (명령 실행용)
#include "../DCMotor/DCMotor.h"
#include "../EC-sensor/EC_module.h"

const char *device_id = "esp32_station_01";

void network_init() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed. Continuing in offline mode...");
  }
}

void upload_integrated_data(float temp, float ec, uint8_t *imgBuf,
                            size_t imgLen) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  http.begin(SERVER_POST_URL);
  http.addHeader("Content-Type", "application/json");

  // 이미지 데이터를 Base64로 인코딩
  String base64Image = "";
  if (imgBuf != NULL && imgLen > 0) {
    base64Image = "data:image/jpeg;base64," + base64.encode(imgBuf, imgLen);
  }

  // JSON 데이터 구성 (메모리 절약을 위해 String 조작 사용)
  String jsonPayload = "{";
  jsonPayload += "\"device_id\":\"" + String(device_id) + "\",";
  jsonPayload += "\"temperature\":" + String(temp, 2) + ",";
  jsonPayload += "\"ec_value\":" + String(ec, 2) + ",";
  jsonPayload += "\"image_data\":\"" + base64Image + "\",";
  jsonPayload += "\"timestamp\":\"\""; // 타임스탬프는 서버측에서 생성 권장
  jsonPayload += "}";

  Serial.println(">> Uploading integrated data...");
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void poll_motor_command() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  http.begin(SERVER_GET_URL);

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.print("Received Command: ");
    Serial.println(payload);

    // 단순 문자열 파싱 (ArduinoJson을 사용하지 않는 경우)
    if (payload.indexOf("\"direction\":\"left\"") > 0) {
      // 좌측 회전 로직 (DCMotor 모니터링 로직에 따라 구현 필요)
      // 현재 DCMotor.cpp에는 'INSPECT' 명령만 있으므로, 상황에 맞게 확장 가능
      startInspection(); // 예시로 INSPECT 시작
    } else if (payload.indexOf("\"direction\":\"stop\"") > 0) {
      stopInspection();
    }
    // 명세에 맞춰 direction, speed, duration 등을 추가 처리할 수 있습니다.
  }
  http.end();
}
