#include <Arduino.h>

const int PUL_PIN = 4; // D2
const int DIR_PIN = 5; // D1

// ==========================================
// 【实验参数设置区 1】固定圈数，修改时间
// ==========================================
const float TARGET_CIRCLES = 3.0;        // 固定转 3 圈
const float TARGET_TIME_SECONDS = 40.0;  // 修改这里：60.0, 40.0, 30.0, 24.0, 20.0
const int RUN_DIR = 1;                   // 1为正转，0为反转
// ==========================================

// 硬件参数 (1600细分, 90:1减速比)
const int MICRO_STEP = 1600;      
const int REDUCTION_RATIO = 90;    
const long STEPS_PER_REV = (long)MICRO_STEP * REDUCTION_RATIO; // 一圈144000步

// 运行参数
long totalSteps = 0;
uint32_t targetFreq = 0;
const uint32_t START_FREQ = 100; // 起步频率
const int RAMP_STEPS = 2000;     // 加速步数，保护偏心臂

void executeExperiment() {
  digitalWrite(DIR_PIN, RUN_DIR ? HIGH : LOW);
  
  for (long i = 0; i < totalSteps; i++) {
    uint32_t currentFreq = targetFreq;

    // 梯形加减速曲线
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
    
    // 每 100 步喂狗一次，防止 ESP8266 重启
    if (i % 100 == 0) yield(); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  // --- 核心计算逻辑 ---
  totalSteps = TARGET_CIRCLES * STEPS_PER_REV;
  // 频率 (Hz) = 总脉冲数 / 总时间(秒)
  targetFreq = totalSteps / TARGET_TIME_SECONDS;

  Serial.println("\n=== 实验 1：固定圈数，控制时间 ===");
  Serial.print("目标圈数: "); Serial.println(TARGET_CIRCLES);
  Serial.print("目标时间: "); Serial.print(TARGET_TIME_SECONDS); Serial.println(" 秒");
  Serial.print("自动计算的运行频率: "); Serial.print(targetFreq); Serial.println(" Hz");
  
  delay(3000); // 准备时间3秒

  Serial.println(">>> 实验开始...");
  uint32_t startTime = millis();
  
  executeExperiment();
  
  uint32_t endTime = millis();
  Serial.print("<<< 实验结束。实际耗时: ");
  Serial.print((endTime - startTime) / 1000.0);
  Serial.println(" 秒");
}

void loop() {
  // 实验执行一次后停止
  delay(100);
  yield();
}