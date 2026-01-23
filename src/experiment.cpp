#include <Arduino.h>

const int PUL_PIN = 4; // D2
const int DIR_PIN = 5; // D1

// 实验参数变量
uint32_t targetFreq = 0;    // 目标频率
int currentDir = 1;         // 当前方向
int rampSteps = 100;        // 加速步数（平滑度）
int stepDelay = 15;         // 加速每步的延迟（ms）

void setup() {
  Serial.begin(115200);
  pinMode(PUL_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  
  Serial.println("\n--- 步进电机力矩/频率实验系统 ---");
  Serial.println("请输入目标频率 (10 - 1000) 后按回车:");
  Serial.println("发送 'f' 切换方向，发送 's' 停止");
}

/**
 * 带加速的运行函数
 * 增加平滑度以减少抖动
 */
void runMotor(uint32_t freq) {
  if (freq < 10) {
    noTone(PUL_PIN);
    return;
  }

  digitalWrite(DIR_PIN, currentDir ? HIGH : LOW);
  
  Serial.print("正在加速至: "); Serial.print(freq); Serial.println(" Hz");

  // 加速过程（梯形平滑）
  uint32_t startF = 20; 
  for (int i = 0; i <= rampSteps; i++) {
    uint32_t f = startF + (freq - startF) * i / rampSteps;
    tone(PUL_PIN, f);
    delay(stepDelay); 
  }
  
  Serial.println("到达稳定转速。");
}

void loop() {
  // 串口指令解析
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "f") {
      currentDir = !currentDir;
      Serial.print("方向切换为: "); Serial.println(currentDir ? "正向" : "反向");
      runMotor(targetFreq); // 切换方向后重新启动
    } 
    else if (input == "s") {
      targetFreq = 0;
      noTone(PUL_PIN);
      Serial.println("紧急停止");
    } 
    else if (input.toInt() > 0) {
      targetFreq = input.toInt();
      runMotor(targetFreq);
    }
  }

  // 这里的 loop 不加 delay，保证串口响应速度
}