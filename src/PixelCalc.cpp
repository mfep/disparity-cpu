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


float windowMeanVector(const std::vector<Float8>& windowData, int window) {
    __m256 sum = _mm256_set1_ps(0);
    for (const auto& float8 : windowData) {
        sum = _mm256_add_ps(sum, float8.toVec());
    }
    std::array<float, 8> result;
    _mm256_store_ps(&result[0], sum);
    float sumf = 0.f;
    for (auto f : result) {
        sumf += f;
    }
    return sumf / window / window;
}


std::vector<Float8> createWindowCache(const Pixelsf& pixels, int cx, int cy, int window) {
    const auto windowCacheSize = static_cast<unsigned>(((window * window / 8) + 1) * 8);
    const auto windowData = pixels.getWindowData(cx, cy, window, windowCacheSize);
    std::vector<Float8> windowCache(windowCacheSize / 8);
    for (int i = 0; i < windowCache.size(); ++i) {
        windowCache[i] = Float8(&windowData[i * 8]);
    }
    return windowCache;
}

}


const std::vector<Float8>& PixelCalc::getWindowData(int cx, int cy) const {
    return m_windowCache[cy * m_pixels.getWidth() + cx];
}


PixelCalc PixelCalc::calculatePixelCalc(const Pixelsf& pixels, int window) {
    PixelCalc calc(pixels);
    std::vector<float> meanData(pixels.getData().size());

    Logger::startProgress("common data calculation (mean, windows)");
    unsigned index = 0;
    for (int row = 0; row < pixels.getHeight(); ++row) {
        for (int col = 0; col < pixels.getWidth(); ++col) {
            meanData[index] = windowMean(pixels, col, row, window);
            calc.m_windowCache[index++] = createWindowCache(pixels, col, row, window);
        }
    }
    Logger::endProgress();
    calc.m_means = std::make_unique<Pixelsf>(std::move(meanData), pixels.getWidth(), pixels.getHeight());
    return calc;
}


PixelCalc::PixelCalc(const Pixelsf& pixels) noexcept :
    m_pixels        (pixels),
    m_windowCache   (pixels.getData().size())
{
}
