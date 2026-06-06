#include "bms_parser.h"
#include "data_simulator.h"
#include "soc_ekf.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

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

// ==================== 1、BatteryPack 基础校验 ====================
TEST_F(BMSTest, EstimatedRange_NonNegative) {
    BatteryPack pack;
    pack.soc = 50.0f;
    pack.soh = 57.0f;
    
    BatteryPack out = parser->parseFrame(pack);
    EXPECT_GE(out.estimated_range, 0.0f);
}

TEST_F(BMSTest, SOC_Clamp) {
    BatteryPack pack;
    pack.soc = -10.0f;
    pack.soh = 57.0f;

    BatteryPack out = parser->parseFrame(pack);
    EXPECT_GE(out.soc, 0.0f);
}

// ==================== 2️⃣  SocEKF（核心） ====================
TEST(SocEKFTest, DischargingReducesSOC) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    float soc = ekf.update(12.0f, 320.0f, 25.0f);
    EXPECT_LT(soc, 100.0f);
}

TEST(SocEKFTest, ChargingIncreasesSOC) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);
    ekf.update(12.0f, 320.0f, 25.0f); // 先放电

    float soc = ekf.update(-7.0f, 330.0f, 25.0f);
    EXPECT_GT(soc, 0.0f);
}

TEST(SocEKFTest, SOC_NeverExceeds100) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    for (int i = 0; i < 100; ++i) {
        ekf.update(-7.0f, 330.0f, 25.0f);
    }

    float soc = ekf.update(-7.0f, 330.0f, 25.0f);
    EXPECT_LE(soc, 100.0f);
}

// ==================== 3️⃣  BMSParser 健壮性 ====================
TEST_F(BMSTest, ParseFrame_InvalidInput_NoCrash) {
    BatteryPack pack{};
    pack.soc = NAN;
    pack.soh = -1.0f;
    pack.total_voltage = 0.0f;

    EXPECT_NO_THROW({
                    BatteryPack out = parser->parseFrame(pack);
                    (void)out;
                    });
}

// ==================== 4️⃣  模拟器 ====================
TEST_F(BMSTest, Simulator_GeneratesValidFrames) {
    BatteryPack pack = simulator->generateFrame();

    EXPECT_GT(pack.total_voltage, 250.0f);
    EXPECT_LE(pack.total_voltage, 420.0f);
    EXPECT_EQ(pack.cells.size(), 96);
}

// ==================== 5️⃣  日志 / 边界（防御性） ====================
TEST_F(BMSTest, FaultDetection_Imbalance) {
    BatteryPack pack;
    pack.max_cell_voltage = 3.8f;
    pack.min_cell_voltage = 2.9f;

    auto faults = parser->detectFaults(pack);
    EXPECT_FALSE(faults.empty());
}
// ==================== BMS 故障注入 ====================
TEST(BMSFaultTest, OverVoltageFault) {
    BMSParser parser;
    BatteryPack pack;

    pack.total_voltage = 500.0f;  // ⚠️ 过压
    pack.max_cell_voltage = 4.25f;
    pack.min_cell_voltage = 3.30f;
    pack.max_temperature = 45.0f;
    pack.min_temperature = 25.0f;

    auto faults = parser.detectFaults(pack);
    EXPECT_FALSE(faults.empty());
}

TEST(BMSFaultTest, UnderVoltageFault) {
    BMSParser parser;
    BatteryPack pack;

    pack.total_voltage = 220.0f;  // ⚠️ 欠压
    pack.max_cell_voltage = 3.60f;
    pack.min_cell_voltage = 2.50f;

    auto faults = parser.detectFaults(pack);
    EXPECT_FALSE(faults.empty());
}

TEST(BMSFaultTest, OverCurrentFault) {
    BMSParser parser;
    BatteryPack pack;

    pack.total_current = 300.0f;  // ⚠️ 过流
    pack.max_cell_voltage = 3.65f;
    pack.min_cell_voltage = 3.30f;

    auto faults = parser.detectFaults(pack);
    EXPECT_FALSE(faults.empty());
}

TEST(BMSFaultTest, CellImbalanceFault) {
    BMSParser parser;
    BatteryPack pack;

    pack.max_cell_voltage = 3.90f;
    pack.min_cell_voltage = 2.80f;  // ⚠️ 压差过大

    auto faults = parser.detectFaults(pack);
    EXPECT_FALSE(faults.empty());
}

TEST(BMSFaultTest, TemperatureImbalanceFault) {
    BMSParser parser;
    BatteryPack pack;

    pack.max_temperature = 70.0f;
    pack.min_temperature = 25.0f;  // ⚠️ 温差过大

    auto faults = parser.detectFaults(pack);
    EXPECT_FALSE(faults.empty());
}

// ==================== EKF 数值稳定性 ====================
TEST(SocEKFTest, MonotonicDischarge) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    float last_soc = 100.0f;
    for (int i = 0; i < 100; ++i) {
        float soc = ekf.update(12.0f, 320.0f, 25.0f);
        EXPECT_LE(soc, last_soc) << "SOC should not increase during discharge";
        last_soc = soc;
    }
}

TEST(SocEKFTest, MonotonicCharge) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    // 先放到低 SOC
    for (int i = 0; i < 100; ++i) {
        ekf.update(12.0f, 320.0f, 25.0f);
    }

    float last_soc = ekf.update(12.0f, 320.0f, 25.0f);
    for (int i = 0; i < 100; ++i) {
        float soc = ekf.update(-7.0f, 330.0f, 25.0f);
        EXPECT_GE(soc, last_soc) << "SOC should not decrease during charge";
        last_soc = soc;
    }
}

TEST(SocEKFTest, SOC_NeverOutOfRange) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    for (int i = 0; i < 200; ++i) {
        float soc = ekf.update(-7.0f, 330.0f, 25.0f);
        EXPECT_GE(soc, 0.0f);
        EXPECT_LE(soc, 100.0f);
    }
}

TEST(SocEKFTest, NoJitterAroundFull) {
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    // 先冲到满
    for (int i = 0; i < 200; ++i) {
        ekf.update(-7.0f, 330.0f, 25.0f);
    }

    std::vector<float> samples;
    for (int i = 0; i < 20; ++i) {
        samples.push_back(ekf.update(-7.0f, 330.0f, 25.0f));
    }

    for (size_t i = 1; i < samples.size(); ++i) {
        EXPECT_NEAR(samples[i], samples[i - 1], 1e-3f)
            << "SOC should not jitter near full";
    }
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
