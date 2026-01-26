#include <Arduino.h>

// ==========================================
// 【配置项】只需修改这里
// 1 : 一直正转
// 2 : 一直反转
// 0 : 停止
#define RUN_MODE 1 
// ==========================================

const int PUL_PIN = 4; // D2
const int DIR_PIN = 5; // D1

// --- 沿用你测试最稳的参数 ---
const uint32_t TARGET_FREQ = 300;  // 最高转速
const uint32_t START_FREQ = 150;   // 起步频率
const int RAMP_STEPS = 150;        // 加速步数

void setup() {
  Serial.begin(115200);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  Serial.println("\n--- 模式化连续旋转 (沿用精准转速参数) ---");

  if (RUN_MODE == 0) {
    noTone(PUL_PIN);
    Serial.println("当前模式：停止");
    return;
  }

  // 1. 设置方向
  // 模式1为正向(HIGH)，模式2为反向(LOW)
  digitalWrite(DIR_PIN, (RUN_MODE == 1) ? HIGH : LOW);
  Serial.println(RUN_MODE == 1 ? "方向：一直正转" : "方向：一直反转");

  // 2. 模拟你原代码的【起步加速过程】
  // 我们在 setup 中先手动跑完这 150 步加速，确保 40cm 臂能拉起来
  Serial.println("正在执行起步加速...");
  for (int i = 0; i < RAMP_STEPS; i++) {
    // 线性计算当前频率
    uint32_t currentFreq = START_FREQ + (TARGET_FREQ - START_FREQ) * i / RAMP_STEPS;
    
    // 计算半周期延迟（微秒）
    uint32_t halfPeriod = 500000 / currentFreq;

    // 手动发送脉冲
    digitalWrite(PUL_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(PUL_PIN, LOW);
    delayMicroseconds(halfPeriod);
    
    if (i % 50 == 0) yield(); // 喂狗
  }

  // 3. 达到最高速后，移交给硬件定时器进行【无限巡航】
  // tone 函数产生的频率非常稳定，适合长时间运行
  tone(PUL_PIN, TARGET_FREQ);
  Serial.println("起步完成，进入巡航状态。");
}

void loop() {
  // 因为是“一直转”，tone() 已经在后台运行了
  // loop 保持空即可，这样可以把 100% 的 CPU 留给后台任务，最稳定
  delay(100);
  yield(); 
}