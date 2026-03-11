#include <Arduino.h>

const int PUL_PIN = 4; // D2
const int DIR_PIN = 5; // D1

// ==========================================
// 【参数修改区】固定圈数，控制时间
// ==========================================
const float TARGET_CIRCLES = 3.0;        // 目标圈数 (比如 3 圈)
const float TARGET_TIME_SECONDS = 20;  // 目标时间 (秒)
const int RUN_DIR = 1;                   // 1: 正转, 0: 反转
// ==========================================

// --- 严格沿用成功的物理参数 ---
const int MICRO_STEP = 1600;      
const int REDUCTION_RATIO = 3;    // 【关键修复】减速比 3:1
const long STEPS_PER_REV = (long)MICRO_STEP * REDUCTION_RATIO; // 4800步一圈

// --- 沿用成功的起步参数 ---
const uint32_t START_FREQ = 300;  // 脆弹起步频率
const int RAMP_STEPS = 300;       // 加减速步数

// 全局运行变量
long totalSteps = 0;
uint32_t calculatedMaxFreq = 0;

void rotateArm(long steps, uint32_t maxFreq, int dir) {
  digitalWrite(DIR_PIN, dir ? HIGH : LOW);
  
  for (long i = 0; i < steps; i++) {
    uint32_t currentFreq = maxFreq;

    // --- 快速梯形加减速逻辑 (严格复刻) ---
    if (i < RAMP_STEPS) {
      currentFreq = START_FREQ + (maxFreq - START_FREQ) * i / RAMP_STEPS;
    } 
    else if (i > steps - RAMP_STEPS) {
      currentFreq = START_FREQ + (maxFreq - START_FREQ) * (steps - i) / RAMP_STEPS;
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
  
  // 计算总步数：圈数 * 4800
  totalSteps = TARGET_CIRCLES * STEPS_PER_REV;
  
  // 计算需要的巡航频率：总步数 / 总时间
  calculatedMaxFreq = totalSteps / TARGET_TIME_SECONDS;

  // 安全保护：如果你设置的时间太短，导致算出来的频率超过 1500Hz（极易堵转），强行限制
  if (calculatedMaxFreq > 1500) {
    calculatedMaxFreq = 1500;
    Serial.println("【警告】计算出的速度太快！已强制降为安全上限 1500Hz");
  }
  // 如果算出来的最高速度比你的起步速度还低，说明时间设得太长了，直接匀速
  if (calculatedMaxFreq < START_FREQ) {
    calculatedMaxFreq = START_FREQ; 
  }

  Serial.println("\n=== 实验 A：固定圈数 ===");
  Serial.print("目标: "); Serial.print(TARGET_CIRCLES); Serial.println(" 圈");
  Serial.print("期望耗时: "); Serial.print(TARGET_TIME_SECONDS); Serial.println(" 秒");
  Serial.print("自动计算运行频率: "); Serial.print(calculatedMaxFreq); Serial.println(" Hz");
  
  delay(100); // 准备时间

  Serial.println(">>> 运行开始");
  uint32_t startTime = millis();
  
  // 执行旋转
  rotateArm(totalSteps, calculatedMaxFreq, RUN_DIR);
  
  uint32_t endTime = millis();
  Serial.print("<<< 运行结束。实际耗时: ");
  Serial.print((endTime - startTime) / 1000.0);
  Serial.println(" 秒");
}

void loop() {
  // 只执行一次
  delay(100);
  yield();
}