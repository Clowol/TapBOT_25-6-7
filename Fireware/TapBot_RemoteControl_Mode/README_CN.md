# TapBot 遥控模式固件维护说明

## 1. 工程概述

本工程是 TapBot 机械臂遥控模式下的 STM32F107 固件，基于 STM32F10x 标准外设库，使用 Keil MDK-ARM 编译。

当前主要功能：

- USART3 遥控器控制链路。
- USART2 上位机 / ROS 2 二进制通讯协议。
- 遥控器与上位机双控制源动态切换。
- 宇树 M8010 电机控制，伸缩关节实际反馈来自 CAN 编码器。
- 飞特 SMS 系列舵机控制。ID0-ID2 作为位置舵机，ID3 作为连续旋转速度舵机。
- 云台速度控制，预留维特 IMU 反馈用于云台闭环。
- 周期性向上位机反馈机械臂状态、执行器回显和诊断信息。

## 2. 编译入口

Keil 工程文件：

```text
Project\RemoteControl_Mode.uvprojx
```

目标 Target：

```text
RemoteControl_Mode
```

最近使用的工具链：

```text
D:\Work-tool\Keil_v5\UV4\uVision.com
ARMCC V5.06 update 6
```

芯片型号：

```text
STM32F107VC
```

## 3. 目录结构

| 目录 | 作用 |
| --- | --- |
| `App/` | 应用入口、周期调度、全局配置和应用层声明。 |
| `App/CPU/` | STM32 中断模板和标准库配置头文件。 |
| `BSP/` | 板级支持包：延时、定时器、LED、开关量、ADC、编码器、Wit IMU适配、系统辅助函数。 |
| `Comm/` | 通讯驱动：USART、CAN1/CAN2、环形缓冲区、串口接收驱动。 |
| `Protocols/` | 协议层：遥控器解析、上位机解析、ROS2包装、反馈打包。 |
| `Control/` | 控制源分配器和通用控制数学函数。 |
| `Actuators/` | 执行器应用层驱动：飞特 SMS 舵机、云台、宇树 M8010。 |
| `Trdlib/GO-M8010-6/` | 宇树 M8010 第三方协议定义和辅助代码。 |
| `Trdlib/wit_c_sdk/` | 维特 CAN IMU SDK 文件。 |
| `Drivers/CMSIS/` | CMSIS Core、STM32F10x 设备文件和启动文件。 |
| `FWlib/` | STM32F10x 标准外设库。 |
| `Project/` | Keil 工程、列表文件、调试配置和构建输出。 |

## 4. 主运行流程

入口文件：

```text
App\main.c
```

主循环中主要调用：

```text
Rmt_CommProc();
ROS2_CommProc();
FunctionProce();
```

周期调度文件：

```text
App\function.c
```

典型链路：

```text
USART DMA/IDLE 中断
-> uart_rx_driver 环形缓冲区
-> 协议解析
-> control_dispatcher 控制分配
-> 执行器驱动
-> 周期反馈到上位机
```

## 5. 通讯链路

### 5.1 USART2：上位机 / ROS 2

| 文件 | 作用 |
| --- | --- |
| `Comm/usart.c` | USART2 DMA 收发和中断处理。 |
| `Comm/uart_rx_driver.c` | 按端口管理串口接收环形缓冲区。 |
| `Protocols/ros2_comm.c` | ROS2/上位机通讯包装层。 |
| `Protocols/upper_comm.c` | 解析 `0xA5` 二进制帧并校验 CRC16-CCITT。 |
| `Protocols/upper_feedback.c` | 打包反馈帧 `0x80`、`0x81`、`0x82`。 |
| `Control/control_dispatcher.c` | 将有效上位机命令分发到控制层或执行器层。 |

上位机帧格式：

```text
0xA5 + CMD_ID + PAYLOAD_LEN + PAYLOAD + CRC_L + CRC_H
```

CRC：

```text
CRC16-CCITT，初值 0xFFFF，多项式 0x1021
范围：CMD_ID + PAYLOAD_LEN + PAYLOAD
字节序：低字节在前
```

### 5.2 USART3：遥控器

| 文件 | 作用 |
| --- | --- |
| `Comm/usart.c` | USART3 DMA 接收和 IDLE 中断。 |
| `Comm/uart_rx_driver.c` | 将接收字节写入 USART3 环形缓冲区。 |
| `Protocols/rmt_comm.c` | 遥控器帧状态机组帧。 |
| `Protocols/rmt_data.c` | 遥控器数据校验、解析和控制量更新。 |

遥控器链路与上位机链路相互独立。遥控模式下，控制分配器继续使用遥控器数据作为有效控制源。

## 6. 源文件作用归纳

### 6.1 App

| 文件 | 作用 |
| --- | --- |
| `App/main.c` | 硬件初始化和主循环。初始化 USART、CAN、定时器、控制分配器、舵机默认模式、云台、编码器和 IMU。 |
| `App/function.c` | 周期任务调度和调试辅助函数，周期发送上位机反馈。 |
| `App/function.h` | 应用层共享声明和公共 include。 |
| `App/app_config.h` | 工程核心参数：波特率、缓冲区大小、执行器常量、调试开关等。 |
| `App/CPU/stm32f10x_conf.h` | 标准外设库配置头文件。 |
| `App/CPU/stm32f10x_it.*` | 标准中断模板；当前未加入 Keil 编译列表。 |

### 6.2 BSP

| 文件 | 作用 |
| --- | --- |
| `BSP/sys.*` | GPIO 位操作、调试打印宏和系统辅助定义。 |
| `BSP/delay.*` | 基于 SysTick 的延时函数。 |
| `BSP/timer.*` | 定时器初始化和周期计数。 |
| `BSP/led.*` | 工作状态 LED 控制。 |
| `BSP/switch.*` | 开关量、限位开关、电磁阀/离合 IO 控制。 |
| `BSP/encoder.*` | M8010 伸缩关节 CAN 编码器命令和反馈解析。 |
| `BSP/wit_imu.*` | 维特 IMU CAN 反馈适配，用于后续云台闭环。 |
| `BSP/adc.*` | ADC 初始化和采样辅助；当前不是机械臂主流程核心。 |
| `BSP/board_config.h` | 板级配置别名。 |
| `BSP/project_macros.h` | 通用宏和打印包装。 |

### 6.3 Comm

| 文件 | 作用 |
| --- | --- |
| `Comm/usart.*` | USART1/2/3/UART4/UART5 初始化、DMA 收发和中断处理。 |
| `Comm/ring_buf.*` | 通用环形缓冲区。 |
| `Comm/uart_rx_driver.*` | USART2/USART3 接收缓冲区管理。 |
| `Comm/can1.*` | CAN1 初始化、发送和接收处理。用于 M8010 与编码器链路。 |
| `Comm/can2.*` | CAN2 初始化和接收处理。用于 Wit IMU 链路。 |

### 6.4 Protocols

| 文件 | 作用 |
| --- | --- |
| `Protocols/rmt_comm.*` | 从 USART3 环形缓冲区组装遥控器数据帧。 |
| `Protocols/rmt_data.*` | 遥控器数据解析和遥控控制变量更新。 |
| `Protocols/ros2_comm.*` | 上位机通讯包装。 |
| `Protocols/upper_comm.*` | USART2 上位机协议解析。 |
| `Protocols/upper_feedback.*` | STM32 到上位机的反馈帧打包。 |
| `Protocols/up_data.*` | 旧上层协议文件。当前上位机主链路不是它，但 `up_data.c` 仍在 Keil 工程中。 |
| `Protocols/assist_data.*` | 旧辅助板通讯协议。当前未加入 Keil 编译。 |

### 6.5 Control

| 文件 | 作用 |
| --- | --- |
| `Control/control_dispatcher.*` | 运行时控制源仲裁和上位机命令分发。 |
| `Control/control_math.*` | 控制映射和通用数学辅助函数。 |
| `Control/device.*` | 旧设备控制聚合模块，服务发动机/发电机类模块；当前未加入 Keil 编译。 |
| `Control/lft_engine.*` | 旧左发动机控制逻辑；当前不属于机械臂主流程。 |
| `Control/rgt_engine.*` | 旧右发动机控制逻辑；当前不属于机械臂主流程。 |
| `Control/generator.*` | 旧发电机启停控制逻辑；当前不属于机械臂主流程。 |

### 6.6 Actuators

| 文件 | 作用 |
| --- | --- |
| `Actuators/steer_data.*` | 飞特 SMS 舵机驱动。支持 SyncWrite、混合模式、前三个位置舵机和 ID3 连续旋转速度控制。 |
| `Actuators/yushu_motor_data.*` | 宇树 M8010 命令生成和遥控/上位机速度执行。 |
| `Actuators/ptz_data.*` | 云台命令封装和控制过程。 |

### 6.7 第三方与官方库

| 文件/目录 | 作用 |
| --- | --- |
| `Trdlib/GO-M8010-6/` | 宇树 M8010 协议结构和辅助实现。 |
| `Trdlib/wit_c_sdk/` | 维特 IMU SDK 和寄存器定义。 |
| `FWlib/` | STM32F10x 标准外设库。 |
| `Drivers/CMSIS/` | CMSIS Core、设备文件和启动文件。当前目标使用 `startup_stm32f10x_cl.s`。 |

## 7. 当前上位机反馈

| 反馈 ID | 名称 | 周期 | 作用 |
| --- | --- | --- | --- |
| `0x80` | `ARM_STATE` | 50 ms | 控制源、故障标志、编码器长度/速度、舵机状态、IMU 状态。 |
| `0x81` | `ACTUATOR_ECHO` | 50 ms | 回显 M8010、SMS 舵机和云台命令。 |
| `0x82` | `DIAG` | 500 ms | 遥控器/上位机在线状态和诊断标志。 |

协议文档保存位置：

```text
E:\TapBOT_26\Fireware\assests
```

## 8. 疑似无用或历史遗留文件

以下文件暂未删除，仅列为待你确认的清理候选。

高置信历史遗留候选：

| 文件 | 原因 |
| --- | --- |
| `Protocols/assist_data.c` / `assist_data.h` | 旧辅助板通讯协议，未加入 Keil，引用关系基本只在自身内部。 |
| `Control/device.c` / `device.h` | 旧设备控制聚合模块，服务发动机/发电机链路，未加入 Keil。 |
| `Control/lft_engine.c` / `lft_engine.h` | 旧左发动机控制逻辑，不属于当前机械臂主链路。 |
| `Control/rgt_engine.c` / `rgt_engine.h` | 旧右发动机控制逻辑，不属于当前机械臂主链路。 |
| `Control/generator.c` / `generator.h` | 旧发电机控制逻辑，不属于当前机械臂主链路。 |

需要谨慎确认的候选：

| 文件 | 原因 |
| --- | --- |
| `Protocols/up_data.c` / `up_data.h` | 旧上层协议。`up_data.c` 仍在 Keil 工程中，但当前有效上位机协议是 `upper_comm.c` + `upper_feedback.c`。确认没有旧符号依赖后，可考虑从 Keil 移除。 |
| `App/CPU/stm32f10x_it.c` / `stm32f10x_it.h` | 标准中断模板存在，但当前未加入 Keil。若以后要把中断集中迁回 `stm32f10x_it.c`，则应保留。 |
| `Drivers/CMSIS/Device/` 下其他 startup 文件 | 当前 STM32F107 使用 `startup_stm32f10x_cl.s`。其他启动文件是不同容量/系列的 CMSIS 备选文件，可保留作官方库参考。 |

删除前请确认：是否还需要保留旧发动机/发电机/辅助板协议作为历史参考或未来迁移素材。

## 9. 维护注意事项

- USART2 承载上位机二进制协议，正常运行时不要输出文本调试信息。临时调试请使用 `APP_USART2_TEXT_DEBUG`。
- USART 中断中只做字节搬运，不放协议解析和执行器业务逻辑。
- 上位机 payload 长度不要超过 `UPPER_PAYLOAD_MAX_LEN`。
- M8010 闭环控制应使用 `BSP/encoder.c` 的编码器反馈，而不是只看发送出去的速度命令。
- 云台角度闭环前，应先检查反馈中的 `imu_valid`。
- SMS 舵机底层仍遵循飞特 SMS 协议，多字节字段低字节在前。
