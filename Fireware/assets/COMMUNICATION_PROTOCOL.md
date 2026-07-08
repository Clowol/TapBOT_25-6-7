# TapBot Lower-Level Communication Protocol

This document describes the current firmware protocol between:

- Main board and ROS2 upper computer
- Main board and end-effector sub-board

All multi-byte numeric fields are little-endian unless otherwise noted.

## 1. Main Board <-> ROS2 Upper Computer

Physical link: USART2, 115200 baud.

Frame format:

| Byte | Field |
|---:|---|
| 0 | `0xA5` frame header |
| 1 | `cmd_id` |
| 2 | `payload_len` |
| 3..N | payload |
| N+1 | `crc_low` |
| N+2 | `crc_high` |

CRC: CRC16-CCITT, initial value `0xFFFF`, polynomial `0x1021`.

CRC input range:

```text
cmd_id + payload_len + payload
```

The frame header `0xA5` is not included in CRC.

Maximum payload length: 32 bytes.

### 1.1 Upper Computer Commands

| ID | Name | Payload |
|---:|---|---|
| `0x01` | `SET_SOURCE` | `u8 source`; `0=remote`, `1=upper` |
| `0x02` | `SET_ALL` | `s16 yushu_speed_x10`, `s16 steer_axis_x10`, `u8 steer_enable_mask`, `u8 ptz_ud_cmd`, `u8 ptz_lr_cmd`, `u8 ptz_ud_speed`, `u8 ptz_lr_speed`, optional `u8 clutch_cmd` |
| `0x10` | `YUSHU_SPEED` | `s16 speed_x10`, optional `u8 clutch_cmd` |
| `0x11` | `EXEC_YUSHU_CAN` | `u8 id`, `u8 mode`, `s16 W_x10`, `s16 T_x100`, `s16 K_W_x1000` |
| `0x12` | `YUSHU_LENGTH` | `s32 target_length_mm`, optional `u16 max_speed_x10` |
| `0x20` | `STEER_SYNC` | `s16 steer_axis_x10`, `u8 enable_mask` |
| `0x21` | `EXEC_STEER_SPEED` | Format A: `u8 enable_mask`, `s16 speed0`, `s16 speed1`, `s16 speed2`, `s16 speed3`; Format B: `u8 enable`, `s16 rotate_speed` |
| `0x22` | `EXEC_STEER_POSITION` | `u8 enable_mask`, `s16 pos0`, `s16 pos1`, `s16 pos2`, optional `s16 pos3`, `u16 run_time`, `u16 speed` |
| `0x23` | `EXEC_STEER_MODE` | `u8 mode`; current firmware sets the default mixed mode |
| `0x30` | `PTZ` | `u8 up_down_cmd`, `u8 left_right_cmd`, `u8 up_down_speed`, `u8 left_right_speed` |
| `0x31` | `EXEC_PTZ` | Same as `PTZ`; immediate execution |
| `0x32` | `EXEC_PTZ_ANGLE` | `s16 yaw_deg_x100`, `s16 pitch_deg_x100`, `u16 tolerance_x100`, `u8 speed` |
| `0x7F` | `HEARTBEAT` | Empty payload |

PTZ direction values:

| Value | Meaning |
|---:|---|
| `0` | stop |
| `1` | up |
| `2` | down |
| `3` | left |
| `4` | right |

Control source values:

| Value | Meaning |
|---:|---|
| `0` | remote controller |
| `1` | upper computer |

Upper-computer watchdog timeout: 50 ticks at 10 ms, approximately 500 ms.

### 1.2 Main Board Feedback

Feedback frame uses the same `0xA5` frame format and CRC rule.

| ID | Name | Nominal period | Payload |
|---:|---|---|---|
| `0x80` | `ARM_STATE` | 100 ms | `u16 seq`, `u32 time_ms`, `u8 active_source`, `u8 system_state`, `u16 fault_flags`, `s32 length_mm`, `s16 line_speed_mm_s`, `u8 limit_switch`, `u8 clutch_state`, `s16 servo_pos0`, `s16 servo_pos1`, `s16 servo_pos2`, `s16 rotate_servo_speed`, `s16 imu_yaw_x100`, `s16 imu_pitch_x100`, `u8 imu_valid` |
| `0x81` | `ACTUATOR_ECHO` | 100 ms | `s16 yushu_speed_x10`, `s16 cmd_W_x10`, `s16 cmd_T_x100`, `s16 cmd_KW_x1000`, `u8 motor_mode`, `s16 steer_speed0`, `s16 steer_speed1`, `s16 steer_speed2`, `s16 steer_speed3`, `u8 ptz_ud_cmd`, `u8 ptz_lr_cmd`, `u8 ptz_ud_speed`, `u8 ptz_lr_speed`, `u8 encoder_valid`, `u8 steer_mode`, `u8 ptz_status` |
| `0x82` | `DIAG` | 500 ms, sent through the 50 ms feedback scheduler | `u8 rmt_online`, `u8 upper_online`, `u8 encoder_valid`, `u8 imu_valid`, `u8 reserved`, `u16 limit_switch`, `u16 fault_flags` |

Fault flags:

| Bit | Meaning |
|---:|---|
| 0 | upper computer timeout |
| 1 | remote controller power-off or communication timeout |
| 2 | CAN1 / encoder communication timeout |
| 3 | PTZ status error |
| 4 | CAN2 / IMU timeout or IMU invalid |

## 2. Main Board <-> End-Effector Sub-Board

Physical link: UART5 / RS485, 115200 baud.

Frame format:

| Byte | Field |
|---:|---|
| 0 | `0xA6` frame header |
| 1 | `node_id`, currently `0x01` |
| 2 | `cmd_id` |
| 3 | `payload_len` |
| 4..N | payload |
| N+1 | `crc_low` |
| N+2 | `crc_high` |

CRC: CRC16-CCITT, initial value `0xFFFF`, polynomial `0x1021`.

CRC input range:

```text
node_id + cmd_id + payload_len + payload
```

The frame header `0xA6` is not included in CRC.

Maximum payload length: 32 bytes.

### 2.1 Main Board Commands To Sub-Board

| ID | Name | Payload |
|---:|---|---|
| `0x01` | `HEARTBEAT` | Empty |
| `0x02` | `ESTOP` | Empty |
| `0x03` | `STOP` | Empty |
| `0x04` | `SET_MODE` | `u8 mode` |
| `0x20` | `SERVO123_SPEED` | `s16 speed0`, `s16 speed1`, `s16 speed2` |
| `0x21` | `SERVO123_POSITION` | `s16 pos0`, `s16 pos1`, `s16 pos2`, `u16 run_time`, `u16 speed` |
| `0x30` | `AUTO_START` | `u16 forward_ms`, `u16 reverse_ms`, `s16 speed` |
| `0x31` | `AUTO_STOP` | Empty |
| `0x40` | `PUSHROD` | Defined but not currently packed by the main-board sender |

Sub-board mode values:

| Value | Meaning |
|---:|---|
| `0` | remote speed mode |
| `1` | upper position mode |
| `2` | automatic task mode |
| `3` | emergency-stop mode |

### 2.2 Sub-Board Feedback To Main Board

| ID | Name | Payload |
|---:|---|---|
| `0x80` | `ACK` | `u8 state`, `u8 status`, `u8 ack_cmd_id` |
| `0x81` | `STATE` | `u8 state`, `u8 error` |
| `0x82` | `SERVO` | Defined; currently not parsed by the main board |
| `0x83` | `ERROR` | `u8 error` |

Sub-board state values:

| Value | Meaning |
|---:|---|
| `0` | unknown |
| `1` | idle |
| `2` | remote |
| `3` | upper |
| `4` | auto running |
| `5` | done |
| `6` | error |
| `7` | emergency stop |

Sub-board error values:

| Value | Meaning |
|---:|---|
| `0` | none |
| `1` | CRC error |
| `2` | length error |
| `3` | timeout |
| `4` | sensor error |
| `5` | pushrod error |
| `6` | servo error |

The main board stores the latest sub-board ACK command ID through `SubBoard_LinkGetLastAckCmd()`.

