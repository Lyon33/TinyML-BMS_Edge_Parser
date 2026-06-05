#include <gtest/gtest.h>
#include "bms_parser.h"
#include "data_simulator.h"
#include <memory>

// 测试 Fixture
class BMSTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<BMSParser>();
        simulator = std::make_unique<DataSimulator>();
    }

    std::unique_ptr<BMSParser> parser;
    std::unique_ptr<DataSimulator> simulator;
};

// ==================== 故障检测测试 ====================
TEST_F(BMSTest, DetectFaults_Imbalance) {
    BatteryPack pack;
    pack.max_cell_voltage = 3.8f;
    pack.min_cell_voltage = 2.9f;   // 压差过大

    auto faults = parser->detectFaults(pack);
    EXPECT_FALSE(faults.empty());
    if (!faults.empty()) {
        EXPECT_EQ(faults[0], "P0A02_Cell_Voltage_Imbalance");
    }
}

TEST_F(BMSTest, DetectFaults_Normal) {
    BatteryPack pack;
    pack.max_cell_voltage = 3.65f;
    pack.min_cell_voltage = 3.28f;

    auto faults = parser->detectFaults(pack);
    EXPECT_TRUE(faults.empty());
}

// ==================== 解析功能测试 ====================
TEST_F(BMSTest, ParseFrame_ValidData) {
    BatteryPack raw;
    raw.soc = 65.0f;
    raw.soh = 57.0f;
    raw.total_voltage = 320.0f;
    raw.max_cell_voltage = 3.65f;
    raw.min_cell_voltage = 3.28f;

    BatteryPack processed = parser->parseFrame(raw);
    EXPECT_GE(processed.estimated_range, 0.0f);
    EXPECT_LE(processed.soc, 100.0f);
    EXPECT_GE(processed.soh, 0.0f);
}

// ==================== 模拟器测试 ====================
TEST_F(BMSTest, Simulator_ConfigLoad) {
    DataSimulator sim;
    BatteryPack pack = sim.generateFrame();
    EXPECT_GT(pack.soc, 0.0f);
    EXPECT_GT(pack.total_voltage, 300.0f);
    EXPECT_EQ(pack.cells.size(), 96);   // 默认电芯数
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
