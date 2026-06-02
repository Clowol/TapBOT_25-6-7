# STM32F107 Firmware Project Layout

## Directory Map

| Directory | Purpose |
| --- | --- |
| `App/` | Application entry, scheduler, debug helpers, project configuration |
| `App/CPU/` | STM32 interrupt and standard-library configuration files |
| `BSP/` | Board support package: delay, timer, GPIO switch, ADC, encoder, LED, sys |
| `Comm/` | Low-level communication drivers: USART, CAN, ring buffer, UART RX driver |
| `Protocols/` | Protocol parsing and data mapping: remote, upper computer, ROS2 wrapper |
| `Control/` | Control allocation, control math, and higher-level control logic |
| `Actuators/` | Application-level actuator drivers for SMS servos, PTZ, and Yushu wrapper |
| `Trdlib/GO-M8010-6/` | Third-party Unitree M8010 protocol/library files |
| `Drivers/CMSIS/` | CMSIS core/device/startup files |
| `FWlib/` | STM32F10x Standard Peripheral Library |
| `Docs/` | Protocol documents and generated reference materials |
| `Project/` | Keil project, build objects, listing files, and debug configuration |

## Keil Entry

Open or build:

```text
Project\TEST.uvprojx
```

The project include path now points to the root-level folders above. The old
`USER` wrapper directory and old `CMSIS/CoreSupport` / `CMSIS/DeviceSupport`
layout are no longer used.

## Current Build Check

The reorganized project has been rebuilt with Keil5:

```text
".\OBJ\TEST.axf" - 0 Error(s), 0 Warning(s).
```
