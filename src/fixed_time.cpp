#include <Arduino.h>

const int PUL_PIN = 4; 
const int DIR_PIN = 5; 

// ==========================================
// 【参数修改区】固定时间，控制圈数
// ==========================================
const float TARGET_TIME_SECONDS = 48.0;  // 固定的 40 秒
const float TARGET_CIRCLES = 3.0;        // 目标圈数 (修改这里: 1, 2, 3...)
const int RUN_DIR = 1;                   
// ==========================================

const int MICRO_STEP = 1600;      
const int REDUCTION_RATIO = 3;    
const long STEPS_PER_REV = (long)MICRO_STEP * REDUCTION_RATIO; 

const uint32_t START_FREQ = 300;  

long totalSteps = 0;
uint32_t calculatedMaxFreq = 0;
long dynamicRampSteps = 0;

void rotateArm(long steps, uint32_t maxFreq, int dir) {
  digitalWrite(DIR_PIN, dir ? HIGH : LOW);
  
  for (long i = 0; i < steps; i++) {
    uint32_t currentFreq = maxFreq;

    if (i < dynamicRampSteps) {
      currentFreq = START_FREQ + (maxFreq - START_FREQ) * i / dynamicRampSteps;
    } 
    else if (i > steps - dynamicRampSteps) {
      currentFreq = START_FREQ + (maxFreq - START_FREQ) * (steps - i) / dynamicRampSteps;
    }

    uint32_t halfPeriod = 500000 / currentFreq;

    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(halfPeriod);
    
    if (i % 200 == 0) yield(); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  totalSteps = TARGET_CIRCLES * STEPS_PER_REV;
  calculatedMaxFreq = totalSteps / TARGET_TIME_SECONDS;

  if (calculatedMaxFreq > 1500) {
    calculatedMaxFreq = 1500;
  }
  if (calculatedMaxFreq < START_FREQ) {
    calculatedMaxFreq = START_FREQ; 
  }

  // --- 【关键修改】动态计算加速步数，使其耗时约 0.5 秒 ---
  float avgFreq = (START_FREQ + calculatedMaxFreq) / 2.0;
  dynamicRampSteps = avgFreq * 0.5; 

  if (dynamicRampSteps > totalSteps / 2) {
    dynamicRampSteps = totalSteps / 3; 
  }

  Serial.println("\n=== 实验 B：固定时间 (0.5s 极速起步版) ===");
  Serial.print("期望耗时: "); Serial.print(TARGET_TIME_SECONDS); Serial.println(" 秒");
  Serial.print("目标: "); Serial.print(TARGET_CIRCLES); Serial.println(" 圈");
  Serial.print("自动计算运行频率: "); Serial.print(calculatedMaxFreq); Serial.println(" Hz");
  Serial.print("加速分配步数: "); Serial.println(dynamicRampSteps);
  
  delay(100); 

  Serial.println(">>> 运行开始");
  uint32_t startTime = millis();
  
  rotateArm(totalSteps, calculatedMaxFreq, RUN_DIR);
  
  uint32_t endTime = millis();
  Serial.print("<<< 运行结束。实际耗时: ");
  Serial.print((endTime - startTime) / 1000.0);
  Serial.println(" 秒");
}

void loop() {
  delay(100);
  yield();
}