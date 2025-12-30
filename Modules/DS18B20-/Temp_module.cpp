#include "Temp_module.h"
#include <DallasTemperature.h>
#include <OneWire.h>

// 전역 객체 생성 (이 파일 내부에서만 실제 객체를 관리)
// 'static'을 붙이거나 .cpp 파일 안에 두면 다른 파일과의 충돌을 막을 수
// 있습니다. 여기서는 간단하게 전역으로 선언하되, 헤더에는 노출치 않았습니다.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// 초기화 함수 구현
void temp_init() { sensors.begin(); }

// 읽기 함수 구현
float temp_read() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}
