# STM32F107 工程维护说明

## 1. 工程概述

本工程基于 STM32F107 和 STM32 标准外设库，当前支持：

- 遥控器控制路径
- ROS 2 上位机串口协议路径
- 宇树 M8010 电机控制
- 飞特 SMS 系列舵机 SyncWrite 控制
- 云台控制
- 遥控器和上位机两种控制源动态切换

Keil 工程入口：

```text
Project\TEST.uvprojx
```


## 2. 目录结构

| 目录 | 说明 |
| --- | --- |
| `App/` | 应用入口、周期调度、全局配置、调试函数 |
| `App/CPU/` | STM32 中断文件和标准库配置文件 |
| `BSP/` | 板级驱动，如定时器、延时、LED、ADC、编码器、开关量 |
| `Comm/` | 通信驱动，如 USART、CAN、环形缓冲区、串口接收驱动 |
| `Protocols/` | 协议层，如遥控器协议、上位机协议、ROS2 兼容包装 |
| `Control/` | 控制分配器、控制数学、控制源切换逻辑 |
| `Actuators/` | 执行器应用层控制，如宇树电机、飞特舵机、云台 |
| `Trdlib/GO-M8010-6/` | 宇树 M8010 第三方协议库 |
| `Drivers/CMSIS/` | CMSIS Core、Device、启动文件 |
| `FWlib/` | STM32F10x 标准外设库 |
| `Docs/` | 协议文档和维护资料 |
| `Project/` | Keil 工程文件、OBJ、LIST、DebugConfig |

## 3. 主循环与周期任务

主循环位于：

```text
App\main.c
```

周期调度位于：

```text
App\function.c
```

当前主要任务：

| 周期 | 函数 | 作用 |
| --- | --- | --- |
| 主循环 | `FunctionProce()` | 处理串口、遥控器、云台等非固定周期任务 |
| 10 ms | `ControlDispatcher_Update()` | 更新当前控制源和统一控制命令 |
| 30 ms | `YushuMotor_ControlProc()` | 宇树 M8010 电机控制 |
| 50 ms | `PTZ_ControlProc()` | 云台控制 |
| 100 ms | `Steer_ControlProc()` | 飞特 SMS 舵机速度控制 |
| 1 s | `LED_WorkCtrlFun()` / `function_prtf()` | 状态灯和调试打印 |

## 4. 通信链路

### 4.1 遥控器 USART3

相关文件：

```text
Comm\uart_rx_driver.c
Protocols\rmt_comm.c
Protocols\rmt_data.c
```

处理流程：

```text
USART3 中断接收字节
-> uart_rx_driver 写入环形缓冲区
-> Rmt_CommProc() 组帧
-> Proce_Rmtdata() 校验并解析
-> RmtDataExchange() 更新遥控控制量
-> ControlDispatcher_Update() 生成统一控制命令
```

遥控器有效帧会刷新：

```text
CntRxRmt = 1000
RmtOutCommunFlg = 0
RmtPwrOffFlg = 0
```

若遥控器失联，控制分配器会强制生成安全零命令。

### 4.2 上位机 USART2

相关文件：

```text
Comm\uart_rx_driver.c
Protocols\upper_comm.c
Control\control_dispatcher.c
```

协议帧格式：

```text
0xA5 + CMD_ID + PAYLOAD_LEN + PAYLOAD + CRC_L + CRC_H
```

CRC：

```text
CRC16-CCITT
初值: 0xFFFF
多项式: 0x1021
范围: CMD_ID + PAYLOAD_LEN + PAYLOAD
字节序: 低字节在前
```

协议文档：

```text
Docs\UpperProtocol\STM32_ROS2_Upper_Protocol.xlsx
Docs\UpperProtocol\STM32_ROS2_Upper_Protocol_Manual.docx
```

## 5. 控制源分配

核心文件：

```text
Control\control_dispatcher.c
Control\control_dispatcher.h
```

控制源：

| 控制源 | 枚举 |
| --- | --- |
| 遥控器 | `CTRL_SRC_RMT` |
| 上位机 | `CTRL_SRC_UPPER` |

遥控器路径保持默认优先。上位机立即执行类命令只有在 `g_RmtUpManCtrlMode == UP_MODE` 时执行。

上位机超时：

```text
CTRL_UPPER_TIMEOUT_TICKS_10MS = 50
```

约 500 ms 未收到有效上位机命令，则退回遥控路径。

## 6. 宇树 M8010 电机维护点

相关文件：

```text
Actuators\yushu_motor_data.c
Actuators\yushu_motor_data.h
Trdlib\GO-M8010-6\
App\app_config.h
```

当前控制周期为 30 ms。

关键配置：

```c
#define YUSHU_M8010_USE_CAN     1U
#define YUSHU_M8010_CAN_TX_ID   0x01U
#define YUSHU_M8010_ID          0U
#define YUSHU_M8010_MODE_IDLE   0U
#define YUSHU_M8010_MODE_RUN    1U
#define YUSHU_M8010_MODE_STOP   YUSHU_M8010_MODE_RUN
#define YUSHU_RMT_GEAR_STOP_DEADBAND 5.0f
```

遥控模式下的控制链路：

```text
RmtGearValue
-> ControlDispatcher_LoadRemoteCmd()
-> g_ctrl_cmd.yushu_gear_cmd
-> YushuMotor_ControlProc()
-> YushuMotor_SendControl()
-> CAN1
```

安全停机策略：

- 遥控器失联时，控制分配器输出零速度命令。
- 宇树速度接近 0 时，`cmd.W/T/K_W` 清零。
- 遥控模式下，拨杆回中会先经过 `YUSHU_RMT_GEAR_STOP_DEADBAND` 死区，避免机械回中偏差形成持续小速度。
- 宇树速度接近 0 时，`cmd.mode` 切到 `YUSHU_M8010_MODE_STOP`。
- 限位触发时，`cmd.W/T/K_W` 清零并切到 `YUSHU_M8010_MODE_STOP`。

若后续发现停止模式与当前 M8010 CAN 转接协议不一致，优先检查：

```text
App\app_config.h
YUSHU_M8010_MODE_RUN
YUSHU_M8010_MODE_STOP
YUSHU_RMT_GEAR_STOP_DEADBAND
```

## 7. 飞特 SMS 舵机维护点

相关文件：

```text
Actuators\steer_data.c
Actuators\steer_data.h
```

当前支持：

- SMS 位置模式
- SMS 速度模式
- SyncWrite 同步写
- 模式写入去重

注意：

- SMS 多字节数据低字节在前。
- `0x21` 为运行模式地址，代码已记录 `SteerActualMode`，避免重复写模式。
- 位置模式 SyncWrite 从 `0x2A` 连续写 6 字节：位置、时间、速度。

## 8. 云台维护点

相关文件：

```text
Actuators\ptz_data.c
Actuators\ptz_data.h
```

遥控器路径在 `rmt_data.c` 中会直接更新云台方向和速度。

上位机路径通过：

```text
UPPER_CMD_PTZ
UPPER_CMD_EXEC_PTZ
```

## 9. 常见问题排查

### 9.1 宇树电机停止后仍发热

优先检查：

1. `YushuSpeed` 是否已经变为 0。
2. `cmd.W`、`cmd.T`、`cmd.K_W` 是否清零。
3. `cmd.mode` 是否切到 `YUSHU_M8010_MODE_STOP`。
4. 遥控器是否失联但 `RmtGearValue` 仍保持旧值。
5. CAN 转接协议是否使用不同的 idle/run 模式编号。

### 9.2 遥控器断开后执行器仍动作

检查：

```text
RmtPwrOffFlg
RmtOutCommunFlg
CntRxRmt
ControlDispatcher_LoadRemoteCmd()
```

遥控失联时，控制分配器应输出安全零命令。

### 9.3 上位机命令无效

检查：

1. 帧头是否为 `0xA5`。
2. `PAYLOAD_LEN` 是否不超过 32。
3. CRC 是否按 `CMD_ID + PAYLOAD_LEN + PAYLOAD` 计算。
4. 是否已切换到 `UP_MODE`。
5. 立即执行类命令是否满足 payload 长度要求。

## 10. 编译验证

使用 Keil5：

```text
D:\Work-tool\Keil_v5\UV4\uVision.com
```

工程：

```text
Project\TEST.uvprojx
```

最近一次整理后编译结果：

```text
".\OBJ\TEST.axf" - 0 Error(s), 0 Warning(s).
```
