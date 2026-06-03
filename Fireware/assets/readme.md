# TapBOT 下位机通讯协议手册

---

## 一、系统控制源

当前有两个控制源：

### 1. 遥控器模式 `RMT_MODE`

- 遥控器从 **USART3** 输入。
- `Rmt_CommProc()` 从 USART3 环形缓冲区解析遥控帧。
- `RmtDataExchange()` 将遥控通道量转换为工程控制量：
  - `RmtGearValue`：M8010 伸缩速度输入，范围约 **-50 ~ +50**
  - `Rmt_Y1_Value`：舵机 1~3 速度输入，范围约 **-50 ~ +50**
  - `Rmt_PTZ_UpDown` / `Rmt_PTZ_LftRgt`：云台方向
  - `RmtClutchStartStop`：电磁阀/离合控制
- `ControlDispatcher_Update()` 将遥控变量整理成统一的 `g_ctrl_cmd`。
- 执行层读取 `g_ctrl_cmd` 控制 M8010、云台、副板舵机。

### 2. 上位机模式 `UP_MODE`

- 上位机从 **USART2** 输入。
- `ROS2_CommProc()` 实际调用 `Upper_CommProc()`。
- `Upper_CommProc()` 解析 **0xA5 自定义协议帧**。
- 解析成功后调用：`ControlDispatcher_OnUpperPacketReceived(cmd_id, payload, len)`

- 控制分配器根据 `cmd_id` 分发到：

| 功能 | 说明 |
|------|------|
| M8010 CAN 控制 | 电机速度/位置控制 |
| 舵机位置控制 | 舵机 1~3 位置闭环 |
| 云台速度/角度控制 | PTZ 控制 |
| 控制源切换/心跳 | 模式切换与保活 |

> **上位机要真正接管，需要满足两个条件：**
>
> 1. 遥控/系统模式变量 `g_RmtUpManCtrlMode == UP_MODE`
> 2. 上位机心跳未超时

上位机超时时间：

```c
CTRL_UPPER_TIMEOUT_TICKS_10MS = 50  // 约 500 ms
```

> **建议**：上位机以 **50~100 ms** 周期发送心跳 `0x7F`。

---

## 二、主循环调度

主循环核心流程：

```c
while(1)
{
    ROS2_CommProc();     // USART2 上位机协议解析
    FunctionProce();     // 遥控、串口、主副板接收处理

    if(flag10ms)   Function_10ms();
    if(cnt30ms)    Function_30ms();
    if(cnt50ms)    Function_50ms();
    if(cnt100ms)   Function_100ms();
    if(cnt500ms)   Function_500ms();
}
```

### 周期任务说明

| 周期 | 函数 | 作用 |
|------|------|------|
| **10ms** | `Function_10ms()` | `ControlDispatcher_Update()` + `SubBoard_LinkTick10ms()` |
| **30ms** | `Function_30ms()` | `YushuMotor_ControlProc()` + M8010 控制输出 |
| **50ms** | `Function_50ms()` | `PTZ_ControlProc()` + `UpperFeedback_SendArmState()` + `UpperFeedback_SendActuatorEcho()` |
| **100ms** | `Function_100ms()` | 仅遥控模式下执行 `Steer_ControlProc()`（避免上位机位置控制时被遥控速度逻辑覆盖） |
| **500ms** | `Function_500ms()` | `UpperFeedback_SendDiag()` |

---

## 三、遥控模式执行逻辑

遥控模式下，控制链路：

```
USART3 遥控器
  -> Rmt_CommProc()
  -> Proce_Rmtdata()
  -> RmtDataExchange()
  -> ControlDispatcher_Update()
  -> g_ctrl_cmd
  -> 执行层
```

### 各执行器行为

**M8010（伸缩电机）：**

- 使用 `RmtGearValue`，中位死区后映射为 `YushuSpeed`
- `YushuSpeed > 0`：电磁阀闭合 `SW_CLUTCH(ON)`
- `YushuSpeed < 0`：电磁阀松开 `SW_CLUTCH(OFF)`
- `YushuSpeed == 0`：发送停止命令

**舵机 1~3：**

- 遥控模式为**速度模式**
- `Rmt_Y1_Value` 映射为 `SteerRunSpeed`
- 通过主副板协议发给副板：`SUBBOARD_CMD_SERVO123_SPEED`

**舵机 4：**

- 不再由主控直接给速度，作为末端自动流程的一部分
- 遥控通道触发自动流程：
  - 按下：`SUBBOARD_CMD_AUTO_START`
  - 松开：`SUBBOARD_CMD_AUTO_STOP`

**云台：**

- 遥控按键触发上下/左右方向
- 使用默认速度：
  - 上下：`PTZ_UP_DOWN_SPEED_DEFAULT = 0x20`
  - 左右：`PTZ_LEFT_RIGHT_SPEED_DEFAULT = 0x10`

---

## 四、上位机到主控板协议

### 物理接口

```
上位机 <-> 主控板 USART2
波特率：115200
```

### 帧格式

```
0xA5  cmd_id  payload_len  payload...  crc_low  crc_high
```

### CRC

```
CRC16-CCITT
init  = 0xFFFF
poly  = 0x1021
计算范围 = cmd_id + payload_len + payload
CRC 低字节先发
```

### 上位机命令 ID

| cmd_id | 名称 | 作用 |
|--------|------|------|
| `0x01` | `UPPER_CMD_SET_SOURCE` | 切换控制源 |
| `0x02` | `UPPER_CMD_SET_ALL` | 设置统一控制量 |
| `0x10` | `UPPER_CMD_YUSHU_SPEED` | 设置 M8010 速度目标 |
| `0x11` | `UPPER_CMD_EXEC_YUSHU_CAN` | 直接下发 M8010 CAN 控制量 |
| `0x20` | `UPPER_CMD_STEER_SYNC` | 设置舵机速度型统一命令 |
| `0x21` | `UPPER_CMD_EXEC_STEER_SPEED` | 舵机速度命令（当前不建议作为主路径） |
| `0x22` | `UPPER_CMD_EXEC_STEER_POSITION` | 舵机 1~3 位置控制 |
| `0x23` | `UPPER_CMD_EXEC_STEER_MODE` | 设置舵机默认模式 |
| `0x30` | `UPPER_CMD_PTZ` | 设置云台速度命令到统一控制量 |
| `0x31` | `UPPER_CMD_EXEC_PTZ` | 立即执行云台速度控制 |
| `0x32` | `UPPER_CMD_EXEC_PTZ_ANGLE` | 云台角度闭环目标 |
| `0x7F` | `UPPER_CMD_HEARTBEAT` | 上位机心跳 |

### 推荐上位机接管流程

```
1. 周期发送 0x7F 心跳
2. 发送 0x01，payload = 01，切换到上位机模式
3. 周期发送执行命令
4. 若停止控制，发送速度 0 或停止命令
5. 若失联超过 500ms，主控自动回到遥控模式
```

---

## 五、常用上位机 Payload

### 0x01 SET_SOURCE

```
payload[0] = 0  遥控模式
payload[0] = 1  上位机模式
```

### 0x10 YUSHU_SPEED

```
payload[0..1] = speed_cmd_s16_le    // 实际速度量 = speed_cmd_s16 × 0.1
payload[2]    = clutch_cmd          // 可选: 0=松开, 1=闭合, 0xFF=不改变
```

### 0x11 EXEC_YUSHU_CAN

```
payload[0]   = motor_id
payload[1]   = mode
payload[2:3] = W_s16_le       // W = value × 0.1
payload[4:5] = T_s16_le       // T = value × 0.01
payload[6:7] = K_W_s16_le     // K_W = value × 0.001
```

### 0x22 EXEC_STEER_POSITION（推荐 11 字节格式）

```
payload[0]   = enable_mask          // bit0~bit2 对应舵机1~3
payload[1:2] = pos0_s16_le
payload[3:4] = pos1_s16_le
payload[5:6] = pos2_s16_le
payload[7:8] = run_time_u16_le
payload[9:10] = speed_u16_le
```

> **说明**：当前上位机模式下，舵机 1~3 采用位置控制。舵机 4 不由上位机位置控制，仍归末端自动流程。

### 0x31 EXEC_PTZ

```
payload[0] = up_down_cmd
payload[1] = left_right_cmd
payload[2] = up_down_speed
payload[3] = left_right_speed
```

### 0x32 EXEC_PTZ_ANGLE

```
payload[0:1] = yaw_target_s16_le      // deg × 100
payload[2:3] = pitch_target_s16_le    // deg × 100
payload[4:5] = tolerance_u16_le       // deg × 100
payload[6]   = speed
```

---

## 六、主控板到副板通信

### 物理接口

```
主控板 UART5 <-> 副板 USART1/RS485
波特率：115200
```

### 帧格式

```
0xA6  node_id  cmd_id  payload_len  payload...  crc_low  crc_high
```

当前节点：`node_id = 0x01`

### CRC

```
CRC16-CCITT
init  = 0xFFFF
poly  = 0x1021
计算范围 = node_id + cmd_id + payload_len + payload
CRC 低字节先发
```

### 主控发给副板命令

| cmd_id | 名称 | 作用 |
|--------|------|------|
| `0x01` | `HEARTBEAT` | 心跳/查询状态 |
| `0x02` | `ESTOP` | 副板急停 |
| `0x03` | `STOP` | 停止副板动作 |
| `0x04` | `SET_MODE` | 设置副板工作模式 |
| `0x20` | `SERVO123_SPEED` | 舵机 1~3 速度控制 |
| `0x21` | `SERVO123_POSITION` | 舵机 1~3 位置控制 |
| `0x30` | `AUTO_START` | 启动末端自动流程 |
| `0x31` | `AUTO_STOP` | 停止末端自动流程 |
| `0x40` | `PUSHROD` | 预留/推杆控制 |

### 副板反馈

| cmd_id | 名称 | Payload |
|--------|------|---------|
| `0x80` | `ACK` | state, status, ack_cmd_id |
| `0x81` | `STATE` | state, error |
| `0x83` | `ERROR` | error |

### 副板状态

| 值 | 状态 |
|----|------|
| 0 | `UNKNOWN` |
| 1 | `IDLE` |
| 2 | `REMOTE` |
| 3 | `UPPER` |
| 4 | `AUTO_RUNNING` |
| 5 | `DONE` |
| 6 | `ERROR` |
| 7 | `ESTOP` |

### 副板模式

| 值 | 模式 | 说明 |
|----|------|------|
| 0 | `REMOTE_SPEED` | 遥控速度模式 |
| 1 | `UPPER_POSITION` | 上位机位置模式 |
| 2 | `AUTO` | 自动流程 |
| 3 | `ESTOP` | 急停 |

---

## 七、副板自动流程

副板文件：`end_effector_task.c`

### 自动流程逻辑

```
AUTO_START
  -> 推杆伸出
  -> 保压等待
  -> 舵机4正转
  -> 舵机4反转
  -> 检测光电传感器无物体，或达到反转超时
  -> 舵机4停止
  -> 推杆缩回
  -> DONE
  -> IDLE
```

### 遥控/上位机模式对比

**共同点**：舵机 4 都不直接由外部连续控制，统一归副板自动流程控制。外部只负责触发 `AUTO_START` / `AUTO_STOP`。

| 项目 | 遥控模式 | 上位机模式 |
|------|----------|------------|
| 舵机 1~3 | 速度模式 | 位置模式 |
| 舵机 4 | 自动流程 | 自动流程 |

---

## 八、主控反馈给上位机

主控通过 USART2 主动反馈，帧格式同上位机协议：

```
0xA5  fb_cmd_id  payload_len  payload...  crc_low  crc_high
```

### 反馈 ID

| fb_cmd_id | 名称 | 周期 |
|-----------|------|------|
| `0x80` | `ARM_STATE` | 50ms |
| `0x81` | `ACTUATOR_ECHO` | 50ms |
| `0x82` | `DIAG` | 500ms |

### 0x80 ARM_STATE Payload

| 字段 | 类型 | 说明 |
|------|------|------|
| `seq` | u16 | 序列号 |
| `time_ms` | u32 | 时间戳(ms) |
| `ctrl_source` | u8 | 0=遥控, 1=上位机 |
| `system_state` | u8 | 系统状态 |
| `fault_flags` | u16 | 故障标志位 |
| `length_mm` | s32 | M8010 编码器反馈长度 |
| `line_speed_mm` | s16 | 线速度 |
| `limit_switch` | u8 | 限位开关 |
| `clutch_state` | u8 | 离合状态 |
| `servo0_pos` | s16 | 舵机0位置 |
| `servo1_pos` | s16 | 舵机1位置 |
| `servo2_pos` | s16 | 舵机2位置 |
| `servo3_speed` | s16 | 舵机3速度 |
| `yaw_deg_x100` | s16 | 偏航角 ×100 |
| `pitch_deg_x100` | s16 | 俯仰角 ×100 |
| `imu_valid` | u8 | IMU 有效标志 |

### 0x81 ACTUATOR_ECHO Payload

| 字段 | 类型 | 说明 |
|------|------|------|
| `yushu_speed_cmd` | s16 | 速度指令 (×0.1) |
| `cmd_W` | s16 | W 指令 (×0.1) |
| `cmd_T` | s16 | T 指令 (×0.01) |
| `cmd_KW` | s16 | KW 指令 (×0.001) |
| `cmd_mode` | u8 | 指令模式 |
| `servo0_speed` ~ `servo3_speed` | s16 | 舵机 0~3 速度 |
| `ptz_ud_cmd` | u8 | 云台上下指令 |
| `ptz_lr_cmd` | u8 | 云台左右指令 |
| `ptz_ud_speed` | u8 | 云台上下速度 |
| `ptz_lr_speed` | u8 | 云台左右速度 |
| `encoder_valid` | u8 | 编码器有效标志 |
| `steer_actual_mode` | u8 | 舵机实际模式 |
| `ptz_work_status` | u8 | 云台工作状态 |

### 0x82 DIAG Payload

| 字段 | 类型 | 说明 |
|------|------|------|
| `remote_ok` | u8 | 遥控器在线 |
| `upper_alive` | u8 | 上位机在线 |
| `encoder_valid` | u8 | 编码器有效 |
| `imu_valid` | u8 | IMU 有效 |
| `reserved` | u8 | 保留 |
| `limit_switch` | u16 | 限位开关状态 |
| `fault_flags` | u16 | 故障标志位 |

### 故障位 `fault_flags`

| 位 | 含义 |
|----|------|
| bit0 | 上位机心跳超时 |
| bit1 | 遥控器掉线/关机 |
| bit2 | CAN1 通信异常 |
| bit3 | 云台状态异常 |
| bit4 | CAN2/IMU 无效 |

---

## 九、给上位机程序的建议

上位机程序建议分三层写：

```
SerialDriver
  负责串口收发、字节流缓存

TapbotProtocol
  负责 0xA5 帧打包/解析、CRC16-CCITT

ControlNode
  负责业务命令：
  - set_source()
  - heartbeat()
  - set_yushu_speed()
  - set_servo123_position()
  - set_ptz_speed()
  - set_ptz_angle()
  - parse_feedback()
```

### 最小稳定控制周期建议

| 功能 | 建议周期 |
|------|----------|
| 心跳 | 50 ~ 100ms |
| 舵机位置控制 | 20 ~ 50ms |
| M8010 速度控制 | 30 ~ 50ms |
| 云台速度控制 | 50ms |
| 反馈解析 | 收到即解析 |

> **特别注意**：上位机进入控制前先发 `0x01 SET_SOURCE = 1`，之后持续发 `0x7F HEARTBEAT`。否则控制分配器会在约 **500ms** 后回退到遥控模式。
