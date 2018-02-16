#ifndef CPU_GOLDEN_PIXELS_HPP
#define CPU_GOLDEN_PIXELS_HPP


#include <vector>


template<typename T>
class Pixels {
public:
    Pixels(std::vector<T>&& data, unsigned width, unsigned height) noexcept :
            m_data      (data),
            m_width     (width),
            m_height    (height)
    {
        static_assert(std::is_arithmetic<T>::value, "arithmetic type required");
    }

    unsigned getWidth() const noexcept { return m_width; }
    unsigned getHeight() const noexcept { return m_height; }
    const std::vector<T>& getData() const noexcept { return m_data; }

    T get(int row, int col) const noexcept {
        if (row < 0) {
            row = 0;
        } else if ((unsigned)row >= m_height) {
            row = m_height - 1;
        }
        if (col < 0) {
            col = 0;
        } else if ((unsigned)col >= m_width) {
            col = m_width - 1;
        }
        return m_data[m_width * row + col];
    }

private:
    const std::vector<T> m_data;
    const unsigned m_width;
    const unsigned m_height;
};

using Pixelsi = Pixels<int>;
using Pixelsf = Pixels<float>;

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("testing Pixels accessor") {
    Pixels pw ({1, 2, 3, 4, 5, 6, 7, 8, 9}, 3, 3);
    CHECK_EQ(pw.get(0, 0), 1);
    CHECK_EQ(pw.get(0, -1), 1);
    CHECK_EQ(pw.get(-1, 0), 1);
    CHECK_EQ(pw.get(1, -1), 4);
    CHECK_EQ(pw.get(1, 3), 6);
    CHECK_EQ(pw.get(3, 3), 9);
}
#endif


#endif //CPU_GOLDEN_PIXELS_HPP
