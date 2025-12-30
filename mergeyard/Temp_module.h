#ifndef TEMP_MODULE_H
#define TEMP_MODULE_H

#include <Arduino.h>

// 핀 설정 (하드웨어 설정 값은 헤더에서 관리하면 보기 편합니다)
#define ONE_WIRE_BUS 4

// 함수 선언 (사용 가능한 기능 목록)
// 이 함수들은 외부(Main 파일 등)에서 불러다 쓸 수 있습니다.
void temp_init();
float temp_read();

#endif
