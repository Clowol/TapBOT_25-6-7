# TapBot 主控/副控固件审查与控制逻辑

审查日期：2026-07-04  
范围：`TapBot_RemoteControl_Mode`（STM32F107VC 主控）与 `EndEffector_SubBoard`（STM32F103C8 副控）当前工作区版本。

## 1. 结论摘要

两套工程都能由 ARMCC 5.06 update 6 完整生成固件，但“编译通过”不代表当前版本适合直接上机。主控全量重编译为 **0 error / 5 warnings**，副控为 **0 error / 0 warnings**。静态审查发现多项控制安全和功能风险，其中副控舵机与主副板通信共用 USART1、通信失联停机、反馈新鲜度以及自动任务停止语义应在联机测试前优先处理。

## 2. Keil 构建结果

| 工程 | 芯片 | 结果 | 固件占用 |
| --- | --- | --- | --- |
| `RemoteControl_Mode` | STM32F107VC | 0 errors, 5 warnings | Code 35,228 B；RO 880 B；RW 716 B；ZI 4,476 B |
| `EndEffector_SubBoard` | STM32F103C8 | 0 errors, 0 warnings | Code 4,844 B；RO 272 B；RW 40 B；ZI 1,200 B |

主控告警均为未使用符号：`Comm/usart.c` 3 个局部变量、`Comm/can1.c` 的 `io_temp`、`Actuators/yushu_motor_data.c` 的 `Yushu_ScaleFloat()`。

## 3. 问题清单

### P0：上机前应解决

1. **副控的主板 RS485 链路与飞特舵机发送共用 USART1/PA9，且舵机发送不拉高 RS485 DE。**  `BSP/usart.c:10-29` 仅初始化 USART1（PA9/PA10），`Actuators/feetech_servo.c:36-39` 又通过同一个 `SubUart_SendByte()` 发舵机帧，而 DE 只在 `Comm/rs485_link.c:168-177` 的主副板协议发送时控制。按当前代码，舵机帧很可能无法送到舵机，或与主副板链路发生总线冲突。必须结合原理图确认舵机物理接口；通常应为舵机配置独立 UART/半双工收发器。

2. **副控没有主控心跳看门狗。** 主控每约 0.5 s 发送心跳，但副控只响应状态，没有记录最后接收时间或在超时后执行 `StopAll + Pushrod_Stop`。主副板线缆断开时，速度模式舵机、ID3 自动流程或推杆可能继续保持最后状态。

3. **遥控失联停机约需 50.1 s。** `Protocols/rmt_data.c:255` 每次有效帧把 `CntRxRmt` 设为 1000；该计数在约 50.1 ms 的 TIM4 中断中递减（`BSP/timer.c:183-186`），因此失联检测不是 1 s，而约为 50.1 s。对运动机构而言过长。

4. **编码器和 IMU 的 `valid` 只会置 1，不会在 CAN 超时后清零。** CAN 超时仅设置 `CAN1_OutCommunFlg`/`CAN2_OutCommunFlg`；长度 PID 和云台角度闭环仍直接依赖缓存中的 `valid`。断线后系统可能继续使用旧位置/姿态执行闭环。应引入数据时间戳/年龄，并在超时入口使反馈失效且停机。

5. **自动任务停止/报错不会撤销宇树长度闭环目标。** `ArmAutoTask_Stop()` 与 `ArmAutoTask_SetError()` 只停止云台和副控自动任务；此前写入 `UpperCmd` 的 `yushu_length_ctrl_enable` 和目标长度仍可被 30 ms 控制循环持续执行。自动停止应发布完整零速/取消位置环命令。

### P1：高优先级功能与安全问题

6. **上位机心跳会持续“续命”旧运动指令。** `UPPER_CMD_HEARTBEAT` 重置与命令共用的 `UpperTimeoutTicks`；只要心跳存在，旧 `UpperCmd` 可无限保持。链路在线但控制节点卡死时，旧速度仍有效。应分离 link watchdog 与 command watchdog，并要求运动命令周期刷新。

7. **上位机的舵机速度统一命令链路不完整。** `UPPER_CMD_STEER_SYNC` 只更新 `UpperCmd`，但 100 ms 的 `Steer_ControlProc()` 仅在遥控模式执行；`UPPER_CMD_EXEC_STEER_SPEED` 又只写 ID3，随后调用的 `SendSteer_Rotate_SpeedRun()` 是空函数。因此 0x20/0x21 不能按接口语义驱动 ID0-ID2。

8. **上位机失联时自动流程未被显式中止。** 控制源回退只停止云台，没有调用 `ArmAutoTask_Stop()`；若副控自动流程已经启动，它会继续运行到完成或自身超时。

9. **副控 E-stop 不锁存。** `EndEffectorTask_EStop()` 进入 ESTOP，但随后 `AUTO_STOP/STOP` 会调用 `EndEffectorTask_StopAuto()` 并直接回到 IDLE。若急停要求人工复位，应增加独立、明确的 reset 命令，普通 stop/mode 命令不能解除急停。

10. **推杆回缩以 5 s 定时结束作为“完成”，没有到位反馈。** `END_AUTO_PUSHROD_TIMEOUT_MS` 到时后直接进入 DONE，而不是报超时；这无法区分正常到位、堵转和线缆故障。建议增加伸出/缩回限位或电流判据。

11. **CAN1 过滤器在字段赋值前调用初始化函数。** `Comm/can1.c:129-138` 首次 `CAN_FilterInit(&CAN_FilterInitStructure)` 使用未初始化的栈变量，随后才赋值并再次初始化。第一次调用应删除。

### P2：可靠性和维护问题

12. **调度周期存在约 10% 偏差。** 72 MHz 定时器时钟、PSC=7199、ARR=10 时，中断周期为 `(7199+1)×(10+1)/72 MHz = 1.1 ms`；累计 10 次后所谓“10 ms”实际为 11 ms。TIM4 的 ARR=500 对应 50.1 ms。应使用 ARR=9/499，或统一基于测得时基配置 PID 与超时。

13. **`flag10ms` 是布尔标志，主循环阻塞时会丢调度拍。** 改为递增 tick 计数或基于单调时钟追赶任务更稳健。

14. **TIM3/TIM4 同时开启 Update 与 Trigger 中断，但 ISR 只处理/清除 Update。** 若产生 trigger 事件，可能留下未清中断源。当前不需要 Trigger 时应只开 Update。

15. **副控错误码不会在新任务或正常 stop 时清零。** 传感器超时后的 `EndError=4` 会继续随后续正常状态上报。

16. **环形缓冲区溢出被静默忽略。** 主副控 ISR 都忽略 `ring_buf_put()` 的返回值，无法诊断丢帧；应累计 overflow 计数并上报。

17. **主控初始化阶段会向 USART2 上位机二进制链路发送文本/原始字节。** `Encoder_Set_Resp_Fun()` 的 `"transmit Ok"` 和 `Encoder_Read_Pos_Dirc_Fun()` 的原始 CAN 缓冲未受 `APP_USART2_TEXT_DEBUG` 保护，可能干扰上位机帧同步。

18. **状态定义与实现有漂移。** 副控定义了 `WAIT_OBJECT_CLEAR`，实现未进入该状态；注释、README 与当前执行路径也有若干差异。建议由状态表生成文档或至少建立单元测试保持同步。

## 4. 当前控制逻辑

### 4.1 主控启动与周期任务

1. 初始化 72 MHz 系统、LED/NVIC、USART1~5、CAN1/2、开关量、控制分配器、TIM3/4、舵机模式、云台、编码器和 IMU。
2. 主循环持续解析 USART2 上位机帧、USART3 遥控帧和 UART5 副控反馈。
3. 名义 10 ms：控制源仲裁、机械臂自动任务、副控在线计时；每 500 ms 发送副控心跳。
4. 名义 30 ms：宇树 M8010 速度/长度 PID、离合和限位逻辑。
5. 名义 50 ms：上位机状态/执行器/诊断反馈；上位机模式下执行云台速度或角度闭环。
6. 名义 100 ms：仅遥控模式下执行 ID0-ID2 舵机速度命令，并用 D 键边沿触发/停止副控自动流程。

### 4.2 控制源仲裁

- 遥控器 H 通道选择 RMT 或 UP 模式。
- 只有同时满足“请求 UP、已有有效 UpperCmd、上位机在 500 ms 窗口内活跃”时，主控采用上位机命令。
- 其余情况回退到遥控；若遥控被判定失联，则发布零运动和离合关闭。
- 从 UP 回退 RMT 时仅显式停止云台，未完整停止自动任务和所有执行器。

### 4.3 主控自动流程

`ARM_AUTO_START (0x40)` 携带 flags、云台角度、宇树目标长度、ID0-ID2 位置、副控正反转时长/速度和回缩长度：

`MOVE_TO_TARGET → WAIT_ARM → END_AUTO（可选）→ RETRACT（可选）→ DONE`

- `MOVE_TO_TARGET`：一次性发布云台角度、宇树长度目标和 ID0-ID2 位置。
- `WAIT_ARM`：检查编码器有效性、最小稳定时间和目标长度容差；12 s 超时。
- `END_AUTO`：要求副控在线，发送自动启动，并等待副控 DONE/IDLE；20 s 超时。
- `RETRACT`：停止云台/副控，ID0-ID2 回零，宇树回到回缩长度；12 s 超时。
- 任意错误进入 ERROR；当前 ERROR/ABORT 不会自动回到 IDLE。

### 4.4 副控手动与自动流程

- REMOTE_SPEED：ID0-ID2 接收速度命令。
- UPPER_POSITION：ID0-ID2 接收位置、运行时间和速度命令。
- AUTO：`WAIT_OBJECT → PUSHROD_EXTEND → PRESS_HOLD → SERVO_FORWARD → SERVO_REVERSE → PUSHROD_RETRACT → DONE → IDLE`。
- 光电传感器在 5 s 内未检测物体则 ERROR；ID3 正转指定时间，再反转直到物体离开或反转时间到；推杆固定回缩 5 s 后视为完成。

### 4.5 通信协议

- 上位机：`A5 | cmd | len | payload | CRC16-L | CRC16-H`，CRC 覆盖 cmd/len/payload。
- 主副板：`A6 | node=01 | cmd | len | payload | CRC16-L | CRC16-H`，CRC 覆盖 node/cmd/len/payload。
- 主控每约 0.5 s 发心跳；副控用 STATE 应答。命令 0x01~0x7F 为主到副，反馈 0x80~0xFF 为副到主。

## 5. 建议整改顺序

1. 先确认副控原理图并拆分/修正飞特舵机 UART 与 RS485 DE 控制。
2. 建立统一安全停机函数，覆盖 M8010、ID0-ID3、云台、推杆和离合；由遥控、上位机、主副板、CAN 反馈 watchdog 共同调用。
3. 将通信在线与运动命令新鲜度分开计时；超时默认进入 stop，而不是无条件切换到另一个控制源。
4. 为 encoder/IMU/subboard feedback 增加时间戳，并让闭环控制只接受未过期数据。
5. 修复上位机舵机速度链路、定时器 ARR 和 E-stop 复位语义。
6. 建立台架故障注入测试：拔遥控、拔上位机、断 UART5、断 CAN1/2、传感器常高/常低、推杆堵转。

## 6. 审查边界

本次为源代码静态审查、工程配置检查和 Keil 全量构建；未包含原理图、示波器/逻辑分析仪采样、执行器台架或硬件在环验证。与引脚、收发器和传感器极性有关的结论需在硬件资料上最终确认。
