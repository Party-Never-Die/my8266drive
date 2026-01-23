#include <Arduino.h>

const int PUL_PIN = 4; // D2
const int DIR_PIN = 5; // D1

// --- 速度优化参数 ---
const int MICRO_STEP = 1600;      
const int REDUCTION_RATIO = 3;    // 如果是3:1就改回3
const long STEPS_PER_REV = (long)MICRO_STEP * REDUCTION_RATIO; 

// --- 调优这些参数来缩短起步时间 --- 同时也是最高转速设置
const uint32_t TARGET_FREQ = 300;  // 提高最高转速 (从800提升到1500)
const uint32_t START_FREQ = 150;    // 提高起始频率 (从50提升到150，起步更脆)
const int RAMP_STEPS = 150;         // 极大缩小加减速步数 (从2000降到400，加速更猛)

void rotateArm(long totalSteps, uint32_t maxFreq, int dir) {
  digitalWrite(DIR_PIN, dir ? HIGH : LOW);
  
  for (long i = 0; i < totalSteps; i++) {
    uint32_t currentFreq = maxFreq;

    // --- 快速梯形加减速逻辑 ---
    if (i < RAMP_STEPS) {
      // 加速阶段
      currentFreq = START_FREQ + (maxFreq - START_FREQ) * i / RAMP_STEPS;
    } 
    else if (i > totalSteps - RAMP_STEPS) {
      // 减速阶段
      currentFreq = START_FREQ + (maxFreq - START_FREQ) * (totalSteps - i) / RAMP_STEPS;
    }

    uint32_t halfPeriod = 500000 / currentFreq;

    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(halfPeriod);
    
    // 喂狗（保持，防止重启）
    if (i % 200 == 0) yield(); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  Serial.println("\n--- Fast Ramp Start ---");
}

void loop() {
  // 正转
  rotateArm(STEPS_PER_REV, TARGET_FREQ, 1);
  delay(100); 

  // 反转
  rotateArm(STEPS_PER_REV, TARGET_FREQ, 0);

  // 回到原点停 1 秒
  Serial.println("Reset Position, Waiting 1s");
  delay(100); 
}