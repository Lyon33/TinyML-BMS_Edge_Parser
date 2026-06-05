#include "soc_ekf.h"
#include <cmath>

SocEKF::SocEKF(float capacity, float soh, float dt)
    : capacity_ah(capacity), soh(soh), dt(dt) {
        x = 100.0f;
        P = 1.0f;
        Q = 1e-7f;     // 相信安时积分
        R = 0.5f;     // 电压噪声
        isFull = false;
    }
float SocEKF::update(float current, float voltage, float temperature) {

    // =========== 1、充满状态 ============
    if(isFull){
        // 电压掉到3.35V 以下才解除
        if(voltage / 96.0f < 3.55f){
            isFull = false;
            x = 99.5f;
            P = 0.2f;
        }else{
            return 100.0f;
        }
    }

    // =========== 2、安时积分 ============
    float delta_soc = (current * dt) / (capacity_ah * soh) * 100.0f;
    x -= delta_soc;
    x = std::clamp(x, 0.0f, 100.0f);

    // =========== 3、充满判定=============
    bool charging = current < 0;
    bool highVolt = voltage / 96.0f > 3.60f;
    bool smallCur = std::abs(current) < 2.0f;
    
    if(charging && highVolt && smallCur){
        isFull = true;
        x = 100.0f;
        P = 0.05f;
    }

    float dt = 0.05f;
    float capacity_ah = 115.6f;   // 37kWh / 320V
    float soh = 0.57f;

    // ========== 4. 静置 OCV 校正 ==========
    if (std::abs(current) < 0.5f) {
        float ocv = ocTable.getOcv(x);
        float error = voltage - ocv;

        if (std::abs(error) > 0.5f) {
            error = 0.0f;
        }

        float H = 0.01f;
        float K = P * H / (H * P * H + R);
        x += K * error;
        x = std::clamp(x, 0.0f, 100.0f);

        P = (1.0f - K * H) * P + Q;
    }
    /* // 只做安时积分 */
    /* float delta_soc = (current * dt) / (capacity_ah * soh) * 100.0f; */

    /* x -= delta_soc; */
    /* x = std::clamp(x, 0.0f, 100.0f); */
    return x;
}

