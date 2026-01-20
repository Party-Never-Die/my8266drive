#include <Arduino.h>

/**
 * 硬件接线说明 (ESP8266 NodeMCU / D1 Mini):
 * GPIO 4  ---> 开发板上的 D2 脚 ---> 接驱动器 PUL (脉冲)
 * GPIO 5  ---> 开发板上的 D1 脚 ---> 接驱动器 DIR (方向)
 * GND     ---> 开发板 GND      ---> 接驱动器 PUL- 和 DIR- (共阴接法)
 */

const int PUL_PIN = 4; // GPIO4 (对应丝印 D2)
const int DIR_PIN = 5; // GPIO5 (对应丝印 D1)

/* 
   硬件参数逻辑 (1600细分, 3:1减速比):
   - 台面转一圈所需脉冲 = 1600 * 3 = 4800 脉冲
   - 台面转 1度 所需脉冲 = 4800 / 360 = 13.33 脉冲/度
   
   速度设置示例:
   - 目标速度 15度/秒 -> 频率 = 15 * 13.33 = 200 Hz
   - 目标速度 30度/秒 -> 频率 = 30 * 13.33 = 400 Hz
*/

/**
 * 设置电机速度（频率）
 * @param freq 脉冲频率 Hz
 */
void motorSetSpeed(uint32_t freq) {
  if (freq < 10) {
    noTone(PUL_PIN); // 频率太低则停止输出，防止电机啸叫
    digitalWrite(PUL_PIN, LOW); // 确保停止时为低电平
  } else {
    // ESP8266 的 tone 函数产生占空比 50% 的 PWM
    tone(PUL_PIN, freq); 
  }
}

/**
 * 设置电机方向
 * @param dir 1正传，0反转
 */
void motorSetDir(uint8_t dir) {
  if (dir == 1) {
    digitalWrite(DIR_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, LOW);
  }
}

void setup() {
  // 初始化串口
  Serial.begin(115200);
  Serial.println("\nStepper Motor Control Started (ESP8266)");

  // 初始化引脚模式
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  // 初始状态关闭
  digitalWrite(PUL_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
}

void loop() {
  // --- 1. 正转 10 秒 ---
  Serial.println("Moving Forward (DIR=1, Speed=200Hz)");
  motorSetDir(1);
  motorSetSpeed(200);   // 启动电机
  delay(10000);         // 运行10秒

  // --- 停止缓冲 ---
  Serial.println("Pause 0.5s");
  motorSetSpeed(0);     // 停止脉冲
  delay(500);           // 停顿0.5秒

  // --- 2. 反转 10 秒 ---
  Serial.println("Moving Backward (DIR=0, Speed=200Hz)");
  motorSetDir(0);
  motorSetSpeed(200);   // 启动电机
  delay(10000);         // 运行10秒

  // --- 停止缓冲 ---
  Serial.println("Pause 0.5s");
  motorSetSpeed(0);
  delay(500);
}