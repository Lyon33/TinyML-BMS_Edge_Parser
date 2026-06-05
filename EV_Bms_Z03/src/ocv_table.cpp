#include "ocv_table.h"
#include <algorithm>

OcTable::OcTable() {}

float OcTable::getOcv(float soc) const {

    // 磷酸铁锂典型OCV-SOC表（后期可以改这里）
    static std::vector<OcPoint> table = {
        {0.0f,   2.50f},
        {10.0f,  3.00f},
        {20.0f,  3.22f},
        {50.0f,  3.28f},
        {80.0f,  3.32f},
        {95.0f,  3.45f},
        {100.0f, 3.65f}
    };

    soc = std::clamp(soc, 0.0f, 100.0f);
    if (soc <= table.front().soc) return table.front().voc;
    if (soc >= table.back().soc) return table.back().voc;

    for (size_t i = 0; i < table.size() - 1; ++i) {
        if (soc >= table[i].soc && soc <= table[i+1].soc) {
            float t = (soc - table[i].soc) / (table[i+1].soc - table[i].soc);
            return table[i].voc + t * (table[i+1].voc - table[i].voc);
        }
    }
    return table.back().voc;
}
