# TapBot Remote Control Mode Firmware

## 1. Overview

This repository contains the STM32F107 firmware for TapBot remote-control mode. The project is based on the STM32F10x Standard Peripheral Library and is built with Keil MDK-ARM.

Current main functions:

- Remote controller control path on USART3.
- Upper-computer / ROS 2 binary protocol on USART2.
- Dynamic control-source arbitration between remote controller and upper computer.
- Unitree M8010 actuator control, with CAN encoder feedback used as the stretch-joint feedback.
- Feetech SMS servo control. Servo ID0-ID2 are position servos; servo ID3 is used as a continuous-rotation speed servo.
- PTZ/gimbal velocity control, with Wit IMU feedback reserved for gimbal closed-loop control.
- Periodic upper-computer feedback frames for state, actuator echo, and diagnostics.

## 2. Build Entry

Keil project:

```text
Project\RemoteControl_Mode.uvprojx
```

Target:

```text
RemoteControl_Mode
```

Toolchain used in recent checks:

```text
D:\Work-tool\Keil_v5\UV4\uVision.com
ARMCC V5.06 update 6
```

Main MCU target:

```text
STM32F107VC
```

## 3. Directory Layout

| Directory | Role |
| --- | --- |
| `App/` | Application entry, scheduler, global configuration and application-level declarations. |
| `App/CPU/` | STM32 interrupt template and standard-library configuration headers. |
| `BSP/` | Board support package: delay, timer, LED, switch GPIO, ADC, encoder, Wit IMU wrapper, low-level system helpers. |
| `Comm/` | Low-level communication drivers: USART, CAN1/CAN2, ring buffer, UART RX driver. |
| `Protocols/` | Protocol layer: remote-controller parser, upper-computer parser, ROS2 wrapper, feedback packing. |
| `Control/` | Control-source dispatcher and common control math. |
| `Actuators/` | Application-level actuator drivers: Feetech SMS servo, PTZ/gimbal, Unitree M8010 wrapper. |
| `Trdlib/GO-M8010-6/` | Third-party Unitree M8010 protocol definitions and helper code. |
| `Trdlib/wit_c_sdk/` | Wit Motion CAN IMU SDK files. |
| `Drivers/CMSIS/` | CMSIS core, STM32F10x device files and startup files. |
| `FWlib/` | STM32F10x Standard Peripheral Library. |
| `Project/` | Keil project, listing, debug configuration and generated build output. |

## 4. Main Runtime Flow

Entry point:

```text
App\main.c
```

Main loop tasks:

```text
Rmt_CommProc();
ROS2_CommProc();
FunctionProce();
```

Periodic scheduler:

```text
App\function.c
```

Typical runtime chain:

```text
USART DMA/IDLE interrupt
-> uart_rx_driver ring buffer
-> protocol parser
-> control_dispatcher
-> actuator drivers
-> periodic feedback to upper computer
```

## 5. Communication Paths

### 5.1 USART2: Upper Computer / ROS 2

Relevant files:

| File | Role |
| --- | --- |
| `Comm/usart.c` | USART2 DMA RX/TX and interrupt handling. |
| `Comm/uart_rx_driver.c` | Per-port RX ring-buffer abstraction. |
| `Protocols/ros2_comm.c` | Thin wrapper calling the upper-computer parser. |
| `Protocols/upper_comm.c` | Parses `0xA5` binary frames and verifies CRC16-CCITT. |
| `Protocols/upper_feedback.c` | Packs feedback frames `0x80`, `0x81`, `0x82`. |
| `Control/control_dispatcher.c` | Dispatches valid upper-computer commands to control or actuator layers. |

Frame format:

```text
0xA5 + CMD_ID + PAYLOAD_LEN + PAYLOAD + CRC_L + CRC_H
```

CRC:

```text
CRC16-CCITT, init 0xFFFF, poly 0x1021
Range: CMD_ID + PAYLOAD_LEN + PAYLOAD
Byte order: low byte first
```

### 5.2 USART3: Remote Controller

Relevant files:

| File | Role |
| --- | --- |
| `Comm/usart.c` | USART3 DMA RX and IDLE interrupt. |
| `Comm/uart_rx_driver.c` | Stores received bytes into the USART3 ring buffer. |
| `Protocols/rmt_comm.c` | Remote frame state machine. |
| `Protocols/rmt_data.c` | Remote data validation, decoding and control variable update. |

The remote path is kept independent from the upper-computer protocol. In remote mode, the dispatcher keeps using remote-controller data as the active command source.

## 6. Source File Summary

### 6.1 App

| File | Role |
| --- | --- |
| `App/main.c` | Hardware initialization and main loop. Initializes USART, CAN, timer, dispatcher, servo mode, PTZ, encoder and IMU. |
| `App/function.c` | Periodic task scheduler and debug helpers. Sends upper-computer feedback periodically. |
| `App/function.h` | Shared application declarations and common includes. |
| `App/app_config.h` | Central project parameters: baud rates, buffer sizes, actuator constants and debug switches. |
| `App/CPU/stm32f10x_conf.h` | Standard Peripheral Library configuration header. |
| `App/CPU/stm32f10x_it.*` | Standard interrupt template; currently not included in the Keil build list. |

### 6.2 BSP

| File | Role |
| --- | --- |
| `BSP/sys.*` | GPIO bit helpers, debug print macros and system helper definitions. |
| `BSP/delay.*` | SysTick-based delay functions. |
| `BSP/timer.*` | Timer initialization and periodic tick counters. |
| `BSP/led.*` | Work/status LED control. |
| `BSP/switch.*` | GPIO switch, limit switch and valve/clutch IO handling. |
| `BSP/encoder.*` | CAN encoder command and feedback parser for the M8010 stretch joint. |
| `BSP/wit_imu.*` | Wit IMU CAN feedback adapter for future gimbal closed-loop control. |
| `BSP/adc.*` | ADC initialization and sampling helpers; present in build but currently not central to the main arm flow. |
| `BSP/board_config.h` | Board-level configuration aliases. |
| `BSP/project_macros.h` | Common macros and print wrappers. |

### 6.3 Comm

| File | Role |
| --- | --- |
| `Comm/usart.*` | USART1/2/3/UART4/UART5 initialization, DMA send/receive and IRQ handlers. |
| `Comm/ring_buf.*` | Generic ring buffer used by UART RX drivers. |
| `Comm/uart_rx_driver.*` | USART2/USART3 receive-buffer management. |
| `Comm/can1.*` | CAN1 initialization, send and receive handling. Used by M8010 and encoder path. |
| `Comm/can2.*` | CAN2 initialization and receive handling. Used by Wit IMU path. |

### 6.4 Protocols

| File | Role |
| --- | --- |
| `Protocols/rmt_comm.*` | Remote-controller frame assembly from USART3 ring buffer. |
| `Protocols/rmt_data.*` | Remote-controller data decoding and remote control variable update. |
| `Protocols/ros2_comm.*` | Upper-computer communication wrapper. |
| `Protocols/upper_comm.*` | USART2 upper-computer frame parser. |
| `Protocols/upper_feedback.*` | STM32-to-upper-computer feedback frame packer. |
| `Protocols/up_data.*` | Legacy upper-layer protocol file. It is still included in the Keil project but is not the current USART2 protocol path. |
| `Protocols/assist_data.*` | Legacy auxiliary-board communication. Not included in the current Keil build. |

### 6.5 Control

| File | Role |
| --- | --- |
| `Control/control_dispatcher.*` | Runtime control-source arbitration and upper-computer command dispatch. |
| `Control/control_math.*` | Shared mapping and control math helpers. |
| `Control/device.*` | Legacy device-control aggregation for engine/generator style modules; not in current Keil build. |
| `Control/lft_engine.*` | Legacy left-engine control logic; not in current Keil build. |
| `Control/rgt_engine.*` | Legacy right-engine control logic; not in current Keil build. |
| `Control/generator.*` | Legacy generator start/stop control logic; not in current Keil build. |

### 6.6 Actuators

| File | Role |
| --- | --- |
| `Actuators/steer_data.*` | Feetech SMS servo driver. Supports SyncWrite, mixed mode, first three position servos and ID3 continuous-rotation speed control. |
| `Actuators/yushu_motor_data.*` | Unitree M8010 command generation and remote/upper-computer speed execution. |
| `Actuators/ptz_data.*` | PTZ/gimbal command packing and control procedure. |

### 6.7 Third-party and Vendor Libraries

| File/Directory | Role |
| --- | --- |
| `Trdlib/GO-M8010-6/` | Unitree M8010 protocol structures and helper implementation. |
| `Trdlib/wit_c_sdk/` | Wit Motion IMU SDK and register definitions. |
| `FWlib/` | STM32F10x Standard Peripheral Library. |
| `Drivers/CMSIS/` | CMSIS core/device/startup files. Current target uses `startup_stm32f10x_cl.s`. |

## 7. Current Upper-computer Feedback

| Feedback ID | Name | Period | Purpose |
| --- | --- | --- | --- |
| `0x80` | `ARM_STATE` | 50 ms | Control source, fault flags, encoder length/speed, servo state, IMU state. |
| `0x81` | `ACTUATOR_ECHO` | 50 ms | Echoes M8010, SMS servo and PTZ commands. |
| `0x82` | `DIAG` | 500 ms | Remote/upper online status and diagnostic flags. |

Protocol documents are stored in:

```text
E:\TapBOT_26\Fireware\assests
```

## 8. Suspected Unused or Legacy Files

The following files are not deleted. They are listed for review before cleanup.

High-confidence legacy candidates:

| File | Reason |
| --- | --- |
| `Protocols/assist_data.c` / `assist_data.h` | Legacy auxiliary-board protocol. Not included in Keil and only references itself. |
| `Control/device.c` / `device.h` | Legacy device aggregator for engine/generator modules. Not included in Keil. |
| `Control/lft_engine.c` / `lft_engine.h` | Legacy left-engine control. Not part of current arm-control flow. |
| `Control/rgt_engine.c` / `rgt_engine.h` | Legacy right-engine control. Not part of current arm-control flow. |
| `Control/generator.c` / `generator.h` | Legacy generator control. Not part of current arm-control flow. |

Review-before-cleanup candidates:

| File | Reason |
| --- | --- |
| `Protocols/up_data.c` / `up_data.h` | Legacy upper-layer protocol. `up_data.c` is still included in Keil, but the active upper-computer protocol is `upper_comm.c` + `upper_feedback.c`. Remove from Keil only after confirming no old symbols are required. |
| `App/CPU/stm32f10x_it.c` / `stm32f10x_it.h` | Interrupt template exists but is not included in the current Keil build. Keep if you plan to move IRQ handlers out of `Comm/usart.c` later. |
| Alternate startup files under `Drivers/CMSIS/Device/` | Only `startup_stm32f10x_cl.s` is used for STM32F107 connectivity line. Other startup files are vendor alternatives for different STM32F1 densities. |

Before deleting anything, confirm which of the above legacy features should be kept for reference.

## 9. Maintenance Notes

- Keep USART2 free of text debug output when the upper-computer binary protocol is enabled. Use `APP_USART2_TEXT_DEBUG` only for temporary debugging.
- Do not place parsing or actuator business logic in USART interrupts. Interrupts should only move bytes into buffers.
- Keep all upper-computer protocol payloads at or below `UPPER_PAYLOAD_MAX_LEN`.
- For M8010 closed-loop control, use encoder feedback from `BSP/encoder.c`, not only the sent speed command.
- For PTZ angle closed-loop control, first check `imu_valid` in feedback.
- The SMS servo path follows the Feetech SMS protocol internally. Multi-byte values are little-endian.
