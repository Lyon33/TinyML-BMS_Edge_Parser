#pragma once
#include <vector>

struct OcPoint {
    float soc;
    float voc;
};

class OcTable {
public:
    OcTable();
    float getOcv(float soc) const;
};
