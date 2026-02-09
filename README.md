# GD32E230 Digital Oscilloscope / GD32E230 数字示波器

[English](#english) | [中文](#chinese)

---

<a name="english"></a>
## English

### Project Overview

A simple digital oscilloscope based on the GD32E230C8T6 microcontroller with ST7735 TFT display. This project features a clean, layered architecture inspired by STM32 HAL library design principles.

### Features

- **Real-time Waveform Display** - Live signal visualization on 160x128 TFT screen
- **Voltage Measurement** - Peak-to-peak voltage (Vpp) measurement
- **Frequency Measurement** - Automatic frequency detection
- **PWM Generator** - Configurable PWM output with adjustable frequency and duty cycle
- **Trigger Detection** - Rising edge trigger for stable waveform capture
- **Adjustable Sampling** - Variable sampling step (1-6) for different time bases
- **Multi-page Interface** - Oscilloscope, PWM generator, and art display modes
- **Rotary Encoder Support** - EC11 encoder for intuitive parameter adjustment

### Hardware Specifications

- **MCU**: GD32E230C8T6 (ARM Cortex-M23, 72MHz)
- **Display**: ST7735 TFT LCD (160x128 pixels, SPI interface)
- **ADC**: 12-bit resolution, DMA-based acquisition (1000 samples)
- **Input**: 4 keys + EC11 rotary encoder
- **Output**: 2 status LEDs, PWM output

### Hardware Source

Hardware design based on: https://oshwhub.com/course-examples/yi-qi-yi-biao-jian-yi-shu-zi-shi-bo-qi-she-ji-cha-jian-ban

### Software Architecture

```
Oscilloscope-V2.0/
├── Hardware/          # Hardware abstraction layer (GPIO, SPI, ADC, TFT, Keys)
├── Middle/            # Middle layer (LCD graphics, ADC processing, PWM, Timers)
├── APP/               # Application layer (Oscilloscope task, UI management)
├── User/              # User code (main.c, system initialization)
├── Firmware/          # GD32E23x standard peripheral library
└── Doc/               # Documentation
```

### Key Improvements

The software has been significantly improved from the original hardware design:

- **Enhanced ADC Accuracy** - Longer sampling times, proper calibration, improved voltage scaling
- **Optimized DMA** - Lightweight restart mechanism for continuous acquisition
- **Better Trigger Logic** - Reliable rising edge detection with boundary checks
- **Full-screen Display** - Maximized waveform viewing area (155x128 pixels)
- **Multi-page UI** - Clean separation between oscilloscope, PWM, and art display modes
- **Code Quality** - All comments translated to English, clear layered architecture

### Building the Project

1. Open `Project/Oscilloscope.uvprojx` in Keil MDK (ARM Compiler V6.16+)
2. Build the project (F7)
3. Flash to GD32E230C8T6 via SWD

### Usage

**Oscilloscope Mode:**
- KEY1 (Double Press): Switch between pages
- KEYD (Single Press): Pause/Resume waveform capture
- EC11 Encoder: Adjust sampling step (time base)

**PWM Mode:**
- KEY1 (Single Press): Toggle PWM on/off
- KEY2 (Single Press): Decrease frequency (halve)
- KEY3 (Single Press): Increase duty cycle (+4%)
- KEY3 (Double Press): Decrease duty cycle (-4%)

### License

This project is open source. Hardware design credit goes to the original author. Software improvements are contributed by the project maintainer.

---

<a name="chinese"></a>
## 中文

### 项目简介

基于GD32E230C8T6微控制器和ST7735 TFT显示屏的简易数字示波器。项目采用清晰的分层架构，参考STM32 HAL库的设计理念。

### 功能特性

- **实时波形显示** - 在160x128 TFT屏幕上实时显示信号波形
- **电压测量** - 峰峰值电压(Vpp)测量
- **频率测量** - 自动频率检测
- **PWM信号发生器** - 可配置PWM输出，频率和占空比可调
- **触发检测** - 上升沿触发，稳定捕获波形
- **可调采样** - 可变采样步进(1-6)，适应不同时基
- **多页面界面** - 示波器、PWM发生器和艺术显示模式
- **旋转编码器支持** - EC11编码器，直观调节参数

### 硬件规格

- **主控**: GD32E230C8T6 (ARM Cortex-M23, 72MHz)
- **显示**: ST7735 TFT LCD (160x128像素, SPI接口)
- **ADC**: 12位分辨率，基于DMA采集(1000个采样点)
- **输入**: 4个按键 + EC11旋转编码器
- **输出**: 2个状态LED，PWM输出

### 硬件来源

硬件设计基于：https://oshwhub.com/course-examples/yi-qi-yi-biao-jian-yi-shu-zi-shi-bo-qi-she-ji-cha-jian-ban

### 软件架构

```
Oscilloscope-V2.0/
├── Hardware/          # 硬件抽象层 (GPIO, SPI, ADC, TFT, 按键)
├── Middle/            # 中间层 (LCD图形, ADC处理, PWM, 定时器)
├── APP/               # 应用层 (示波器任务, UI管理)
├── User/              # 用户代码 (main.c, 系统初始化)
├── Firmware/          # GD32E23x标准外设库
└── Doc/               # 文档
```

### 主要改进

软件部分在原始硬件设计基础上进行了大幅改进：

- **增强ADC精度** - 更长的采样时间，正确的校准，改进的电压缩放
- **优化DMA** - 轻量级重启机制，实现连续采集
- **更好的触发逻辑** - 可靠的上升沿检测，带边界检查
- **全屏显示** - 最大化波形显示区域(155x128像素)
- **多页面UI** - 示波器、PWM和艺术显示模式清晰分离
- **代码质量** - 所有注释翻译为英文，清晰的分层架构

### 编译项目

1. 在Keil MDK中打开 `Project/Oscilloscope.uvprojx` (需要ARM Compiler V6.16+)
2. 编译项目 (F7)
3. 通过SWD烧录到GD32E230C8T6

### 使用说明

**示波器模式：**
- KEY1 (双击): 切换页面
- KEYD (单击): 暂停/恢复波形捕获
- EC11编码器: 调节采样步进(时基)

**PWM模式：**
- KEY1 (单击): 开关PWM输出
- KEY2 (单击): 降低频率(减半)
- KEY3 (单击): 增加占空比(+4%)
- KEY3 (双击): 减少占空比(-4%)

### 开源协议

本项目开源。硬件设计归原作者所有，软件改进由项目维护者贡献。

---

## Contributors / 贡献者

- Hardware Design: Original author from oshwhub
- Software Improvements: Eureka_pg

## Screenshots / 截图

*Coming soon / 即将添加*
等我TFT来了再拍照吧，hhh