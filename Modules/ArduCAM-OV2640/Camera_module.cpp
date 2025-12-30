#include "Camera_module.h"

ArduCAM myCAM(OV2640, CS_PIN);
int frameCount = 0;
int failCount = 0;

// 프레임 저장을 위한 내부 버퍼 (320x240 JPEG 기준 약 10-20KB 예상)
// 정적 할당하여 메모리 파편화 방지
uint8_t frameBuffer[30720];

void camera_init() {
  Serial.println("\n=== ArduCAM Ultra-Stable Diagnostic Mode ===");
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(50000);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_PIN);
  SPI.setFrequency(1000000);

  bool spi_ok = false;
  for (int i = 0; i < 5; i++) {
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    if (myCAM.read_reg(ARDUCHIP_TEST1) == 0x55) {
      spi_ok = true;
      break;
    }
    delay(200);
  }

  if (!spi_ok) {
    Serial.println("1. SPI [FAILED]");
    while (1)
      ;
  }
  Serial.println("1. SPI [OK]");

  uint8_t vid, pid;
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);

  if ((vid != 0x26) || (pid != 0x42 && pid != 0x41)) {
    Serial.print("2. I2C [FAILED] ID: ");
    Serial.print(vid, HEX);
    Serial.print(":");
    Serial.println(pid, HEX);
    while (1)
      ;
  }
  Serial.println("2. I2C [OK]");

  myCAM.InitCAM();
  myCAM.set_format(JPEG);
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  delay(2000);
  myCAM.clear_fifo_flag();
  Serial.println("3. System Ready. Starting Diagnostic Stream...\n");
}

bool capture_frame(uint8_t **buf, size_t *len) {
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();

  unsigned long start = millis();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
    if (millis() - start > 2000)
      return false;
    yield();
  }

  uint32_t length = myCAM.read_fifo_length();
  if (length == 0 || length >= sizeof(frameBuffer))
    return false;

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  // FIFO 데이터를 버퍼로 욺김
  for (int i = 0; i < length; i++) {
    frameBuffer[i] = SPI.transfer(0x00);
  }

  myCAM.CS_HIGH();
  myCAM.clear_fifo_flag();

  *buf = frameBuffer;
  *len = length;
  return true;
}

void camera_loop() {
  uint8_t *buf;
  size_t len;
  if (capture_frame(&buf, &len)) {
    frameCount++;
    Serial.print("[Frame ");
    Serial.print(frameCount);
    Serial.print("] Size: ");
    Serial.print(len);

    // 헤더 확인 (FF D8)
    if (buf[0] == 0xFF && buf[1] == 0xD8) {
      Serial.println(" ✅ OK!");
    } else {
      Serial.println(" ❌ BAD");
    }
  } else {
    Serial.println("[Error] Capture Failed");
  }
  delay(500);
}
