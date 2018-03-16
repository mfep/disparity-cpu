#ifndef DISPARITY_CPU_PIXELCALC_HPP
#define DISPARITY_CPU_PIXELCALC_HPP


#include <memory>
#include "Pixels.hpp"
#include "Float8.hpp"


// TODO docs
class PixelCalc {
public:
    static PixelCalc            calculatePixelCalc  (const Pixelsf& pixels, int window);
    const Pixelsf&              pixels              () const { return m_pixels; }
    const Pixelsf&              means               () const { return *m_means; }
    const std::vector<Float8>&  getWindowData       (int cx, int cy) const;

private:
    explicit                    PixelCalc           (const Pixelsf& pixels) noexcept;

    const Pixelsf&                      m_pixels;
    std::unique_ptr<Pixelsf>            m_means;
    std::vector<std::vector<Float8>>    m_windowCache;
};


#endif //DISPARITY_CPU_PIXELCALC_HPP
