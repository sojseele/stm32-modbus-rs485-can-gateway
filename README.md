# STM32F103 Modbus RTU / RS485-CAN 电机控制网关

这是一个基于 STM32F103 的嵌入式通信网关示例项目。

## 项目功能

- 通过 RS485 接收 Modbus RTU 请求
- 支持 Modbus CRC 校验、地址过滤和异常处理
- 支持 0x03、0x06、0x10 功能码
- 将 Modbus 控制寄存器映射为 CAN 电机控制报文
- 将 CAN 电机状态同步回 Modbus 寄存器
- 支持启停、方向、PWM 调速和通信超时保护
- 使用 Keil MDK 工程组织源码

## 主要目录

- `App/`：应用层 Modbus 处理
- `User/`：STM32 外设、RS485、CAN、PWM 和协议实现
- `Libries/`：CMSIS、STM32 HAL 及相关启动文件
- `Project/`：Keil MDK 工程文件
- `Doc/`：协议、接线和帧格式说明
- `CHANGELOG.md`：开发和维护记录
- `LICENSE`：项目自有代码的 MIT 许可证

## 硬件和通信参数

- MCU：STM32F103
- Modbus RTU：RS485，115200-8-N-1
- CAN：500 kbit/s，标准帧
- PWM：1 kHz
- 接收缓冲区：256 字节

## 编译

1. 使用 Keil MDK 打开 `Project/Temprot.uvprojx`。
2. 检查工程中的头文件和源码路径。
3. 选择对应的 STM32F103 芯片型号。
4. 编译并下载到开发板。

## 说明

本项目目前主要用于学习、实验和嵌入式协议网关验证。公开仓库前，请根据实际情况补充硬件照片、测试记录、完整寄存器表和项目许可证。

本项目自有代码使用 MIT License。`Libries/` 中的 ST 官方组件以及其他第三方文件不适用本许可证，请保留其原有版权和许可证说明，并以第三方组件的许可条款为准。
