//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include "lodepng.h"
#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#endif


constexpr int WINDOW = 9;
constexpr int MAX_D = 260 / 4;


std::vector<float> preprocessPixels(const std::vector<unsigned char>& pixels, unsigned& width, unsigned& height)
{
    const float
            R = 0.2126f,
            G = 0.7152f,
            B = 0.0722f;

    unsigned index = 0;
    std::vector<float> resized (pixels.size() / 16 / 3);
    for (unsigned row = 0; row < height; row += 4) {
        for (unsigned col = 0; col < width; col += 4) {
            const unsigned char
                    r = pixels[(row * width + col) * 3],
                    g = pixels[(row * width + col) * 3 + 1],
                    b = pixels[(row * width + col) * 3 + 2];
            resized[index++] = r * R + g * G + b * B;
        }
    }
    width /= 4;
    height /= 4;
    return resized;
}


std::vector<float> loadGreyPixels(const char *filename, unsigned& width, unsigned& height)
{
    std::vector<unsigned char> pixels;
    unsigned error = lodepng::decode(pixels, width, height, filename, LCT_RGB);
    if (error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    } else {
		std::cout << "loading '" << filename << "' was successful" << std::endl;
	}
    return preprocessPixels(pixels, width, height);
}


template<typename T>
void savePixels(const std::vector<T>& pixels, unsigned width, unsigned height)
{
    std::vector<unsigned char> convertedPixels(pixels.size());
    unsigned i = 0;
    for (auto f : pixels) {
        convertedPixels[i++] = static_cast<unsigned char>(f);
    }
    unsigned error = lodepng::encode("im0_resized.png", convertedPixels, width, height, LCT_GREY, 8);
    if (error) {
        std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
}


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

    T get(int row, int col) const noexcept
    {
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

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("testing Pixels accessor")
{
    Pixels pw ({1, 2, 3, 4, 5, 6, 7, 8, 9}, 3, 3);
    CHECK_EQ(pw.get(0, 0), 1);
    CHECK_EQ(pw.get(0, -1), 1);
    CHECK_EQ(pw.get(-1, 0), 1);
    CHECK_EQ(pw.get(1, -1), 4);
    CHECK_EQ(pw.get(1, 3), 6);
    CHECK_EQ(pw.get(3, 3), 9);
}
#endif


float windowMean(const Pixels<float>& pixels, int cx, int cy)
{
    const int D = WINDOW / 2;
    float sum = 0.0f;
    for (int row = cy - D; row <= cy + D; ++row) {
        for (int col = cx - D; col <= cx + D; ++col) {
            sum += pixels.get(row, col);
        }
    }
    return sum / static_cast<float>(WINDOW * WINDOW);
}

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("check if windowMean is correct")
{
    Pixels pw ({77,63,31,29,8,17,72,9,92,43,8,57,83,35,78,71,59,38,39,43,42,22,50,4,56,5,87,86,34,97,95,99,16,0,25,35,23,76,23,45,26,35,90,1,13,39,84,21,94,97,38,98,12,76,58,62,49,22,14,64,80,67,47,94,59,23,68,32,75,100,27,93,70,10,25,93,48,88,78,2,77}, 9, 9);
    CHECK(windowMean(pw, 4, 4) == doctest::Approx(50.8765).epsilon(0.0001));
    CHECK(windowMean(pw, 0, 4) == doctest::Approx(54.4691).epsilon(0.0001));
}
#endif


float windowStd(const Pixels<float> &pixels, int cx, int cy)
{
    const int D = WINDOW / 2;
    const float mean = windowMean(pixels, cx, cy);
    float sum = 0.0f;
    for (int row = cy - D; row <= cy + D; ++row) {
        for (int col = cx - D; col <= cx + D; ++col) {
            const float val = pixels.get(row, col) - mean;
            sum += val * val;
        }
    }
    return sqrtf(sum);
}

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("check if windowStd is correct")
{
    Pixels pw ({77,63,31,29,8,17,72,9,92,43,8,57,83,35,78,71,59,38,39,43,42,22,50,4,56,5,87,86,34,97,95,99,16,0,25,35,23,76,23,45,26,35,90,1,13,39,84,21,94,97,38,98,12,76,58,62,49,22,14,64,80,67,47,94,59,23,68,32,75,100,27,93,70,10,25,93,48,88,78,2,77}, 9, 9);
    CHECK(windowStd(pw, 4, 4, 0) == doctest::Approx(271.6298).epsilon(0.0001));
}
#endif


float calcZncc(const Pixels<float>& pixL, const Pixels<float>& pixR, int cx, int cy, int d, float meanL)
{
    const float meanR = windowMean(pixR, cx - d, cy);

    const int D = WINDOW / 2;
    float sum = 0.0f;
    for (int row = cy - D; row <= cy + D; ++row) {
        for (int col = cx - D; col <= cx + D; ++col) {
            sum += (pixL.get(row, col) - meanL) * (pixR.get(row, col - d) - meanR);
        }
    }
    return sum / windowStd(pixL, cx, cy) / windowStd(pixR, cx - d, cy);
}


int findBestDisparity(const Pixels<float>& pixL, const Pixels<float>& pixR, int cx, int cy, bool invertD)
{
    const float meanL = windowMean(pixL, cx, cy);

    float best_zncc = 0.0f;
    int best_disp = 0;
    for (int disp = 0; disp < MAX_D; ++disp) {
        const float zncc = calcZncc(pixL, pixR, cx, cy, invertD ? -disp : disp, meanL);
        if (zncc > best_zncc) {
            best_zncc = zncc;
            best_disp = disp;
        }
    }
    return best_disp;
}


Pixels<int> calcDepthMap(const Pixels<float>& leftPixels, const Pixels<float>& rightPixels, bool invertD)
{
    const unsigned width = leftPixels.getWidth();
    const unsigned height = leftPixels.getHeight();

    std::vector<int> depthMap(width * height);
    unsigned index = 0;
    for (unsigned row = 0; row < height; ++row) {
        for (unsigned col = 0; col < width; ++col) {
            depthMap[index++] = findBestDisparity(leftPixels, rightPixels, col, row, invertD);
        }
        static int lastProgress = -1;
        const auto currentProgress = static_cast<int>(static_cast<float>(row) / height * 100.f);
        if (lastProgress != currentProgress) {
            std::cout << "progress: " << currentProgress << " %" << std::endl;
            lastProgress = currentProgress;
        }
    }

    return Pixels<int>(std::move(depthMap), width, height);
}


Pixels<int> normalize(const Pixels<int>& input)
{
    std::vector<int> normalizedData(input.getWidth() * input.getHeight());
    for (int i = 0; i < input.getData().size(); ++i) {
        normalizedData[i] = input.getData()[i] * 255 / (MAX_D - 1);
    }
    return Pixels<int>(std::move(normalizedData), input.getWidth(), input.getHeight());
}


#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

int main()
{
    unsigned width, height;
    auto grey1 = loadGreyPixels("im0.png", width, height);
    auto grey2 = loadGreyPixels("im1.png", width, height);

    Pixels<float> greyPx1(std::move(grey1), width, height);
    Pixels<float> greyPx2(std::move(grey2), width, height);

    auto depthNorm = normalize(calcDepthMap(greyPx1, greyPx2, false));
    savePixels(depthNorm.getData(), depthNorm.getWidth(), depthNorm.getHeight());

    return 0;
}

#endif