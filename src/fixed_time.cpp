#include <Arduino.h>

const int PUL_PIN = 4; // D2
const int DIR_PIN = 5; // D1

// ==========================================
// 【实验参数设置区 2】固定时间，修改圈数
// ==========================================
const float TARGET_TIME_SECONDS = 40.0;  // 固定 40 秒
const float TARGET_CIRCLES = 2.0;        // 修改这里：1.0, 2.0, 3.0, 4.0, 5.0
const int RUN_DIR = 1;                   // 1为正转，0为反转
// ==========================================

// 硬件参数 (1600细分, 90:1减速比)
const int MICRO_STEP = 1600;      
const int REDUCTION_RATIO = 90;    
const long STEPS_PER_REV = (long)MICRO_STEP * REDUCTION_RATIO; 

// 运行参数
long totalSteps = 0;
uint32_t targetFreq = 0;
const uint32_t START_FREQ = 100; 
const int RAMP_STEPS = 2000;     

void executeExperiment() {
  digitalWrite(DIR_PIN, RUN_DIR ? HIGH : LOW);
  
  for (long i = 0; i < totalSteps; i++) {
    uint32_t currentFreq = targetFreq;

    if (i < RAMP_STEPS) {
      currentFreq = START_FREQ + (targetFreq - START_FREQ) * i / RAMP_STEPS;
    } 
    else if (i > totalSteps - RAMP_STEPS) {
      currentFreq = START_FREQ + (targetFreq - START_FREQ) * (totalSteps - i) / RAMP_STEPS;
    }

    uint32_t halfPeriod = 500000 / currentFreq;

    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(halfPeriod);
    
    if (i % 100 == 0) yield(); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  // --- 核心计算逻辑 ---
  totalSteps = TARGET_CIRCLES * STEPS_PER_REV;
  targetFreq = totalSteps / TARGET_TIME_SECONDS;

  Serial.println("\n=== 实验 2：固定时间，控制圈数 ===");
  Serial.print("目标时间: "); Serial.print(TARGET_TIME_SECONDS); Serial.println(" 秒");
  Serial.print("目标圈数: "); Serial.println(TARGET_CIRCLES);
  Serial.print("自动计算的运行频率: "); Serial.print(targetFreq); Serial.println(" Hz");
  
  delay(3000);

  Serial.println(">>> 实验开始...");
  uint32_t startTime = millis();
  
  executeExperiment();
  
  uint32_t endTime = millis();
  Serial.print("<<< 实验结束。实际耗时: ");
  Serial.print((endTime - startTime) / 1000.0);
  Serial.println(" 秒");
}

void loop() {
  delay(100);
  yield();
}