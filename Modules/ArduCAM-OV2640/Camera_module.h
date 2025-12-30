#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include "memorysaver.h"
#include <ArduCAM.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

// 핀 설정
#define CS_PIN 5
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define SDA_PIN 21
#define SCL_PIN 22

// 진단용 루프 함수
void camera_init();
void camera_loop();

// 프레임 캡처 함수 (네트워크 전송용)
// 캡처 성공 시 true 반환, buf와 len에 데이터 주소와 길이 저장
bool capture_frame(uint8_t **buf, size_t *len);

#endif
