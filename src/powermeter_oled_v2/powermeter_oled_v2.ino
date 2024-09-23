#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>

// OLED 디스플레이의 회전 방향 설정 (0 = 기본, 1 = 90도 시계 방향, 2 = 180도, 3 = 270도 시계 방향)
#define ROTATION 0

// OLED 디스플레이와 INA219 센서 객체 생성
Adafruit_SSD1306 display;
Adafruit_INA219 ina219;

// 전류 측정을 위해 총 시간과 전류값을 추적할 변수들
uint32_t total_sec = 0; // 총 시간을 초 단위로 저장
float total_mA = 0.0;   // 총 전류를 밀리암페어 단위로 저장

uint8_t counter = 0; // 카운터 변수

// 전원 업데이트와 OLED 디스플레이 갱신 함수
void pixel_show_and_powerupdate() {
  if (counter == 0) {
    update_power_display(); // 전원 정보를 OLED에 표시
  } else {
    counter = (counter+1) % 10; // 카운터를 10마다 0으로 초기화
  }
}

void setup() {
  Serial.begin(115200); // 시리얼 통신 초기화
  while (!Serial) {
    delay(1); // 시리얼이 연결될 때까지 대기
  }

  // INA219 센서 초기화
  if (!ina219.begin()) {
    Serial.println("INA219 칩을 찾을 수 없습니다.");
    while (1) { delay(10); } // 오류 발생 시 무한 대기
  }

  // INA219 센서를 16V, 400mA 범위로 설정
  ina219.setCalibration_16V_400mA();

  // OLED 디스플레이 설정
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Wire.setClock(400000); // I2C 속도 설정

  // 디스플레이 지우기 및 초기화
  display.clearDisplay();
  display.display();

  // 디스플레이 회전 설정
  display.setRotation(ROTATION);

  // 텍스트 크기 및 색상 설정
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void loop() {
  pixel_show_and_powerupdate(); // 전원 정보 갱신 및 디스플레이 업데이트
}

// 전원 정보를 OLED에 표시하는 함수
void update_power_display() {
  // INA219 센서에서 전압과 전류 값을 읽어옴
  float shuntvoltage = ina219.getShuntVoltage_mV();
  float busvoltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();

  // 부하 전압과 전력을 계산
  float loadvoltage = busvoltage + (shuntvoltage / 1000);
  float power_mW = loadvoltage * current_mA;
  (void)power_mW;

  // 총 전류와 시간 업데이트
  total_mA += current_mA;
  total_sec += 1;
  float total_mAH = total_mA / 3600.0;
  (void)total_mAH;

  // OLED 디스플레이 업데이트
  display.clearDisplay();
  display.setCursor(0, 0);

  // 전압과 전류를 표시
  printSIValue(loadvoltage, "V:", 2, 10);
  display.setCursor(0, 16);
  printSIValue(current_mA / 1000.0, "A:", 5, 10);

  display.display(); // 디스플레이에 출력
}

// SI 단위로 값을 출력하는 함수
void printSIValue(float value, const char* units, int precision, int maxWidth) {
  // 값이 1 미만일 때 milli 접두사를 사용
  if (fabs(value) < 1.0) {
    display.print('m');
    maxWidth -= 1;
    value *= 1000.0;
    precision = max(0, precision-3);
  }

  // 단위 출력
  display.print(units);
  maxWidth -= strlen(units);

  // 음수 값을 위한 공간 확보
  if (value < 0.0) {
    maxWidth -= 1;
  }

  // 값의 자리수를 계산
  int digits = ceil(log10(fabs(value)));
  if (fabs(value) < 1.0) {
    digits = 1; // 값이 1보다 작을 경우 0을 위한 자리수 확보
  }

  // 자리가 부족할 경우 대시(-)로 채우기
  if (digits > maxWidth) {
    for (int i = 0; i < maxWidth; ++i) {
      display.print('-');
    }
    if (value < 0.0) {
      display.print('-');
    }
    return;
  }

  // 표시할 실제 소수점 이하 자리수 계산
  int actualPrecision = constrain(maxWidth - digits - 1, 0, precision);

  // 오른쪽 정렬을 위한 공백 추가
  int padding = maxWidth - digits - 1 - actualPrecision;
  for (int i = 0; i < padding; ++i) {
    display.print(' ');
  }

  // 값 출력
  display.print(value, actualPrecision);
}
