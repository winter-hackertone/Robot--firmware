#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H

#include <Arduino.h>

// WiFi 설정 (사용자 수정 필요)
#define WIFI_SSID "Galaxy S22"
#define WIFI_PASSWORD "12345678"

// 서버 API 설정 (사용자 수정 필요)
#define SERVER_POST_URL "http://192.168.127.182:5000/api/sensor/data"
#define SERVER_GET_URL                                                         \
  "http://192.168.127.182:5000/api/motor/command?device_id=esp32_station_01"

// 네트워크 초기화 함수
void network_init();

// 통합 데이터 업로드 함수 (온도, EC, 이미지 버퍼)
void upload_integrated_data(float temp, float ec, uint8_t *imgBuf,
                            size_t imgLen);

// 모터 명령 폴링 함수
void poll_motor_command();

#endif
