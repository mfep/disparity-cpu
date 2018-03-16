#ifndef DISPARITY_CPU_PIXELCALC_HPP
#define DISPARITY_CPU_PIXELCALC_HPP


#include <memory>
#include "Pixels.hpp"


// TODO docs
class PixelCalc {
public:
    static PixelCalc    calculatePixelCalc  (const Pixelsf& pixels, int window);
    const Pixelsf&      pixels              () const { return m_pixels; }
    const Pixelsf&      means               () const { return *m_means; }

private:
    explicit            PixelCalc           (const Pixelsf& pixels) noexcept;

    const Pixelsf&              m_pixels;
    std::unique_ptr<Pixelsf>    m_means;
};


#endif //DISPARITY_CPU_PIXELCALC_HPP
