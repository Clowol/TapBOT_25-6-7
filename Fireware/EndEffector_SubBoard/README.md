# EndEffector_SubBoard

STM32F103 end-effector sub-board framework for the TapBOT arm.

## Hardware role

- RS485 link to STM32F107 main board.
- Photoelectric sensor input.
- Linear pushrod output.
- Four Feetech SMS servos on the end platform.

## Control policy

- Remote mode: servo 1-3 speed commands from main board; servo 4 is reserved for the automatic end-effector workflow.
- Upper-computer mode: servo 1-3 position commands from main board; servo 4 is reserved for the automatic workflow.
- Automatic workflow: object detected, pushrod extends, servo 4 rotates forward, reverses until object clears, pushrod retracts.

## Protocol

Frame: `0xA6 node_id cmd_id payload_len payload crc16_l crc16_h`.
CRC: CRC16-CCITT over `node_id + cmd_id + payload_len + payload`.

Current default pins are placeholders and should be checked against the final PCB:

- USART1 PA9/PA10 for RS485 data.
- PA8 for RS485 DE/RE direction.
- PA0 photoelectric sensor input.
- PB0/PB1 pushrod extend/retract outputs.
