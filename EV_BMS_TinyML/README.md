# TinyML-BMS Edge Parser

**边缘端BMS智能数据解析与TinyML推理系统**  
基于真实高里程电动车场景开发的嵌入式AI项目

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.14+-green)
![TinyML](https://img.shields.io/badge/TinyML-Edge%20AI-orange)
![UDP](https://img.shields.io/badge/UDP-Industrial-orange)

### 项目背景
作者实际驾驶**合创Z03 510km版**纯电动车，累计行驶19.8万公里后，电池出现严重衰减（实际充满一次仅36kWh，续航从510km下降至240km）。为深入理解BMS工作原理并探索边缘AI应用，独立开发此项目。

该项目模拟了**MCU边缘端BMS数据处理 + TinyML推理**场景，高度匹配嵌入式AI、汽车电子、BMS相关岗位需求。

### 核心功能
- 真实车辆BMS数据模拟（96串LFP电池，基于36kWh实际容量）
- BMS协议解析（SOC、SOH、电压、电流、温度、故障码）
- 智能故障诊断与云端日志上报
- **TinyML模拟推理**：基于SOC/SOH/温度预测下一小时SOH衰减
- **工业级UDP接收模块**：多线程、心跳检测、包统计、超时告警
- **CAN协议解析接口**（已预留，支持真实硬件接入）
- CSV完整历史日志记录

### 技术栈
- **语言**：C++17（现代特性、多线程、原子操作）
- **构建**：CMake
- **AI**：TinyML模拟推理（类似TFLite Micro / STM32Cube.AI）
- **通信**：工业级UDP + CAN协议解析接口
- **其他**：JSON动态配置、线程安全设计、优雅退出机制

### 项目架构（文字版）
```
外部硬件/模拟器
↓ (UDP / CAN)
[ Communication Layer  ] ←→ [ Parser Layer  ]
↓           ↓
[ TinyML Inference  ]     [ Fault Diagnosis & Logging  ]
↓           ↓
[ 数据可视化 + CSV日志  ]

```
### 测试UDP方法（等程序运行后，新开一个终端）
- 工业级UDP接收（多线程、端口复用、错误处理、二进制解析）
```
# 测试发送数据
while true; do 
    echo -n -e '\x00\x00\xa0\x42\x00\x00\x70\xc2\x00\x00\x82\x42\x00\x00\xe4\x42\x00\x00\x20\x41\x00\x00\x70\x42' | nc -u -w1 127.0.0.1 8888
    sleep 1
done
```
### 项目结构
- EV_BMS_Parser/
- ├── include/               # 头文件
- ├── src/                   # 源代码
- ├── config/                # 协议配置文件
- ├── third_party/json/      # 第三方库
- ├── data/                  # 测试数据
- ├── bms_log.csv            # 运行日志
- └── README.md

### 架构设计
#### 采用经典分层架构
- Simulator Layer: 真实车辆数据模拟(UDP/CAN数据接收)
- Parser Layer: 协议解析 + 故障诊断
- TinyML Layer: 边缘AI推理模块，模拟MCU端模型部署
- Communication Layer: 工业级UDP接收
- Logger Layer: 数据持久化

### 未来拓展方向
- 支持SocketCAN真实硬件接收 
- 接入真实STM32/ESP32 + TFLite Micro部署
- Web可视化界面（WebSocket + Qt / ImGui）
- 机器学习预测电池衰减趋势
- 支持DBC协议文件解析

### 快速运行
```
cd EV_BMS_TinyML
rm -rf build && mkdir build && cd build
cmake ..
make -j4
./bin/bms_TinyML_Parser
```
