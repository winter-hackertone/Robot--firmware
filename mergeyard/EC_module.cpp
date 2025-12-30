#include "EC_module.h"
#include <HardwareSerial.h>

int readings[SIZE];
int readIndex = 0;
long total = 0;
float averageADC = 0;
unsigned long measurementCount = 0;
int measurementState = 1;
unsigned long lastLogTime = 0;
const unsigned long logInterval = 500; // 로그 출력 간격 (ms)

void ec_init() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  for (int i = 0; i < SIZE; i++) {
    readings[i] = 0;
  }
}

void ec_loop() {
  if (measurementState == 1) {
    int current_adc = read_sensor_adc();
    update_moving_average(current_adc);

    // 로그 출력 빈도 제한
    if (millis() - lastLogTime >= logInterval) {
      check_and_alert(current_adc);
      display_data(current_adc);
      lastLogTime = millis();
    }
  }
}

int read_sensor_adc() { return analogRead(TDS_PIN); }

void update_moving_average(int current_adc) {
  // 0도 유효한 측정값으로 처리할 수 있도록 수정 (건조 상태 또는 순수 증류수 등)
  // if (current_adc == 0) return;
  total = total - readings[readIndex];
  readings[readIndex] = current_adc;
  total = total + current_adc;
  readIndex = (readIndex + 1) % SIZE;
  if (measurementCount < SIZE)
    measurementCount++;
}

void check_and_alert(int current_adc) {
  int valid_divisor = (measurementCount < SIZE) ? measurementCount : SIZE;
  if (valid_divisor > 0) {
    averageADC = (float)total / valid_divisor;
  } else {
    averageADC = 0;
  }

  if (measurementCount >= SIZE) {
    if (current_adc > (averageADC * WARNING_MULTIPLIER) &&
        averageADC > MIN_AVG_THRESHOLD) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("\n!!! [경고] EC 값이 평균보다 급격히 높음 !!!\n");
    }
  } else {
    Serial.print("(안정화 진행 중... ");
    Serial.print(measurementCount);
    Serial.print("/");
    Serial.print(SIZE);
    Serial.println(")");
  }
}

void display_data(int current_adc) {
  float voltage = current_adc * (3.3 / 4095.0);
  Serial.print("Current ADC: ");
  Serial.print(current_adc);
  Serial.print(" | Avg ADC (W=");
  Serial.print(SIZE);
  Serial.print("): ");
  Serial.print(averageADC);
  Serial.print(" | Voltage: ");
  Serial.println(voltage, 2);
}

void set_measurement_state(int new_state) {
  if (new_state == 0) {
    measurementState = 0;
    Serial.println(
        "{\"type\":\"response\",\"module\":\"ec\",\"status\":\"stopped\"}");
  } else if (new_state == 1) {
    measurementState = 1;
    Serial.println(
        "{\"type\":\"response\",\"module\":\"ec\",\"status\":\"started\"}");
  } else {
    Serial.println("{\"type\":\"error\",\"module\":\"ec\",\"message\":"
                   "\"Invalid state input\"}");
  }
}

float get_average_ec() { return averageADC; }
