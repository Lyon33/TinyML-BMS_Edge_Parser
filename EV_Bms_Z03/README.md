# EV BMS Battery Data Parser

**车载动力电池管理系统（BMS）数据解析器** —— 基于真实驾驶经历开发

### 项目背景
- 作者实际驾驶 **合创Z03 510km版**（2023年7月购买，行驶19.8万公里）
- 电池实际可用容量仅剩 **36kWh**（SOH≈57%），续航从510km衰减至240km左右
- 因此开发此工具，用于解析、监控和分析车载BMS数据

### 核心功能
- ✅ 实时BMS数据模拟与解析（支持96串LFP电池）
- ✅ SOC、SOH、总电压、电流、单体电压、温度、故障码等参数处理
- ✅ 异常检测与告警（电压不一致、高温、低SOC等）
- ✅ CSV日志记录
- ✅ 配置化协议支持（可快速适配不同车型）
- ✅ 现代C++17 + CMake + Linux 多线程设计

### 技术栈
- **语言**：C++17（智能指针、多线程、现代语法）
- **构建工具**：CMake
- **系统**：Linux / macOS（arm64）
- **核心设计**：分层架构（Simulator → Parser → Processor → Logger）

### 快速编译运行
```bash
mkdir build && cd build
cmake ..
make -j4
./bms_parser
```

### 测试UDP方法（等程序运行后，新开一个终端）
```
# 测试发送数据
echo -n -e '\x00\x00\xa0\x42\x00\x00\x70\xc2\x00\x00\x82\x42\x00\x00\xe4\x42\x00\x00\x20\x41\x00\x00\x70\x42' | nc -u -w1 127.0.0.1 8888
```

### 项目结构
- EV_BMS_Parser/
- ├── include/          # 头文件
- ├── src/              # 源代码
- ├── config/           # 协议配置文件
- ├── third_party/      # 第三方库
- ├── data/             # 测试数据
- ├── bms_log.csv       # 运行日志
- └── README.md

### 未来拓展方向
- 支持SocketCAN真实硬件接收 
- Web可视化界面（WebSocket + Qt / ImGui）
- 机器学习预测电池衰减趋势
