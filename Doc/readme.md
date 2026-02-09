# GD32E230 Oscilloscope V2.0

## 项目简介 / Project Overview

基于GD32E230C8T6微控制器的简易数字示波器项目。代码经过重构，采用类似STM32 HAL库的分层架构，整体项目逻辑清晰明了。

A simple digital oscilloscope project based on GD32E230C8T6 microcontroller. The code has been refactored following STM32 HAL library style with a clear layered architecture.

## 硬件来源 / Hardware Source

硬件设计基于：https://oshwhub.com/course-examples/yi-qi-yi-biao-jian-yi-shu-zi-shi-bo-qi-she-ji-cha-jian-ban

Hardware design based on the above link.

## 软件改进 / Software Improvements

软件部分由项目维护者改进，主要针对ADC和TFT显示模块进行了优化，提升了性能和精度。

Software improvements by project maintainer, focusing on ADC and TFT display modules for better performance and accuracy.

## 主要特性 / Key Features

- 实时波形显示 / Real-time waveform display
- 电压峰峰值测量 / Voltage peak-to-peak measurement
- 频率测量 / Frequency measurement
- PWM信号发生器 / PWM signal generator
- 可调采样步进 / Adjustable sampling step
- 触发检测 / Trigger detection

## 代码结构 / Code Structure

- **Hardware/** - 硬件抽象层 / Hardware abstraction layer
- **Middle/** - 中间层 / Middle layer
- **APP/** - 应用层 / Application layer
- **User/** - 用户代码 / User code
- **Firmware/** - GD32E23x标准外设库 / GD32E23x standard peripheral library

## 最新更新 / Latest Updates

所有关键注释已翻译为英文，便于国际协作。

All critical comments have been translated to English for international collaboration.
