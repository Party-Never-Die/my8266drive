# my8266drive

基于 `ESP8266 (NodeMCU v2)` 和 `PlatformIO` 的步进电机驱动实验工程，用于验证不同运行策略下的起步、加减速、定圈数和定时间控制。

当前代码面向一个通过脉冲/方向接口驱动的步进电机系统，源码中默认参数假设如下：

- 控制板：ESP8266 `nodemcuv2`
- 框架：Arduino
- 电机侧参数：`1600` 微步、`3:1` 减速比
- 接线：`D1 -> DIR`，`D2 -> PUL`
- 串口波特率：`115200`

## 项目结构

[`platformio.ini`](platformio.ini) 通过不同 `env` 选择单个源文件编译。当前仓库中的主要模式如下：

| PlatformIO 环境 | 源文件 | 作用 |
| --- | --- | --- |
| `test_freq` | [`src/experiment.cpp`](src/experiment.cpp) | 串口输入频率做实验，可切换方向、停止，用于测试频率/力矩表现。 |
| `one_circle` | [`src/auto_circle.cpp`](src/auto_circle.cpp) | 正转一圈后反转一圈，循环往复，包含快速梯形加减速。 |
| `fixed_circles` | [`src/fixed_circles.cpp`](src/fixed_circles.cpp) | 固定圈数，按目标时间反推运行频率。 |
| `fixed_time` | [`src/fixed_time.cpp`](src/fixed_time.cpp) | 固定时间，按目标圈数反推运行频率，并动态计算加速段。 |
| `always_run` | [`src/always_run.cpp`](src/always_run.cpp) | 单方向持续运行，先手动加速，再交给 `tone()` 持续输出脉冲。 |

## 各模式说明

### 1. 频率实验 `test_freq`

[`src/experiment.cpp`](src/experiment.cpp) 通过串口接收命令：

- 输入数字：设置目标频率，范围设计为 `10 - 1000 Hz`
- 输入 `f`：切换正反转
- 输入 `s`：紧急停止

代码使用 `tone()` 产生脉冲，并在切换到目标频率前做线性加速，适合做起步能力和频率区间实验。

### 2. 自动往返 `one_circle`

[`src/auto_circle.cpp`](src/auto_circle.cpp) 每次按 `4800` 步为一圈进行运动：

- 正转一圈
- 短暂停顿
- 反转一圈
- 再次短暂停顿

适合验证机构往返动作是否平稳。

### 3. 持续运行 `always_run`

[`src/always_run.cpp`](src/always_run.cpp) 通过宏 `RUN_MODE` 选择模式：

- `1`：一直正转
- `2`：一直反转
- `0`：停止

启动阶段先手动完成一段加速，再用 `tone()` 进入稳定巡航。

### 4. 固定圈数 `fixed_circles`

[`src/fixed_circles.cpp`](src/fixed_circles.cpp) 中可直接修改：

- `TARGET_CIRCLES`
- `TARGET_TIME_SECONDS`
- `RUN_DIR`

程序会根据总步数和目标时间反推出巡航频率，并带安全上限保护。

### 5. 固定时间 `fixed_time`

[`src/fixed_time.cpp`](src/fixed_time.cpp) 中可直接修改：

- `TARGET_TIME_SECONDS`
- `TARGET_CIRCLES`
- `RUN_DIR`

与固定圈数模式相反，这个模式以运行时长为主约束，并根据平均频率动态分配加速步数。

## 编译与烧录

本工程使用 `PlatformIO`。典型命令如下：

```bash
pio run -e test_freq
pio run -e fixed_time
pio run -e fixed_time -t upload
pio device monitor -b 115200
```

如果你使用 VS Code，安装推荐扩展即可：

- `PlatformIO IDE`

## 参数与运动模型

多个源文件复用了同一套基本假设：

- `MICRO_STEP = 1600`
- `REDUCTION_RATIO = 3`
- `STEPS_PER_REV = 4800`

这意味着 README 中提到的“一圈”指的是经过减速机构后的输出圈数。若你的驱动器细分、减速比或电机机构不同，需要同步修改源码中的常量。

## 已知问题

[`platformio.ini`](platformio.ini) 里的 `always_run` 环境当前写的是：

```ini
build_src_filter = -<*> +<mode_run.cpp>
```

但仓库实际存在的文件是 [`src/always_run.cpp`](src/always_run.cpp)，不是 `mode_run.cpp`。  
如果直接编译 `always_run` 环境，当前配置大概率会失败，需要先把这个文件名对齐。

## 适用场景

这个仓库更像实验工程，而不是通用电机控制库。它适合：

- 调试步进电机在不同频率下的起步和堵转边界
- 验证带减速比机构的往返运动
- 在已知机械参数下快速试验定时长或定圈数运行
- 通过串口做频率/方向切换实验
