#include "PixelCalc.hpp"
#include "Logger.hpp"


namespace {

float windowMean(const Pixelsf& pixels, int cx, int cy, int window) {
    float sum = 0.0f;
    pixels.enumerateWindow(cx, cy, window, [&sum](float value) {
        sum += value;
    });
    return sum / static_cast<float>(window * window);
}

}


PixelCalc PixelCalc::calculatePixelCalc(const Pixelsf& pixels, int window) {
    PixelCalc calc(pixels);
    std::vector<float> meanData(pixels.getData().size());

    Logger::startProgress("mean calculation");
    unsigned index = 0;
    for (int row = 0; row < pixels.getHeight(); ++row) {
        for (int col = 0; col < pixels.getWidth(); ++col) {
            meanData[index++] = windowMean(pixels, col, row, window);
        }
    }
    Logger::endProgress();
    calc.m_means = std::make_unique<Pixelsf>(std::move(meanData), pixels.getWidth(), pixels.getHeight());
    return calc;
}


PixelCalc::PixelCalc(const Pixelsf& pixels) noexcept :
    m_pixels    (pixels)
{
}
