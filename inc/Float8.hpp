#ifndef DISPARITY_CPU_FLOAT8_HPP
#define DISPARITY_CPU_FLOAT8_HPP


#include <array>
#include "immintrin.h"


// TODO docs
class alignas(8) Float8 {
public:
    inline Float8() :
        m_data { 0, 0, 0, 0, 0, 0, 0, 0 }
    {
    }

    explicit inline Float8(const float* data) :
        m_data { 0, 0, 0, 0, 0, 0, 0, 0 }
    {
        for (int i = 0; i < 8; ++i) {
            m_data[i] = data[i];
        }
    }

    inline __m256 toVec() const {
        return _mm256_load_ps(&m_data[0]);
    }

private:
    std::array<float, 8> m_data;
};


#endif //DISPARITY_CPU_FLOAT8_HPP
