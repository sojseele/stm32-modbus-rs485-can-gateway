# Changelog

## 2026-09-03 — Initial Modbus RTU implementation

- Added Modbus RTU slave frame reception and parsing.
- Added CRC validation, address filtering, register handling, and exception responses.

## 2026-09-06 — RS485 communication layer

- Added RS485 half-duplex direction control.
- Improved variable-length UART frame reception and Modbus response handling.

## 2026-09-08 — Transmission reliability

- Refined the Modbus transmit path.
- Improved UART completion handling and RS485 timing for gateway use.

## 2026-09-09 — CAN gateway integration

- Added the CAN control and status protocol.
- Added Modbus register to CAN data mapping.
- Added gateway state handling for control transmission and status response.
- Fixed the CAN protocol header include guard.

## 2026-09-15 to 2026-09-24 — Debugging and validation

- Reviewed CAN ACK and mailbox behavior.
- Investigated busy-state, timeout, callback, and frame-reception handling.
- Checked PWM range validation, motor communication timeout, LED initialization, and debug procedure.
- Documented the complete Modbus → CAN → motor → CAN → Modbus validation flow.

## Planned improvements

- Add a complete register table and protocol test cases.
- Add hardware photos and reproducible test records.
- Improve build and configuration instructions.
