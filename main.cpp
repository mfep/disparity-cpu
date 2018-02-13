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


std::vector<float> preprocessPixels(const std::vector<unsigned char>& pixels, unsigned width, unsigned height)
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
        convertedPixels[i++] = static_cast<unsigned char>(f * 255 / 64);
    }
    unsigned error = lodepng::encode("im0_resized.png", convertedPixels, width, height, LCT_GREY, 8);
    if (error) {
        std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
}


class PixelWrapper {
public:
    PixelWrapper(std::vector<float>&& data, unsigned width, unsigned height) noexcept :
            m_data      (data),
            m_width     (width),
            m_height    (height)
    {
    }

    float get(int row, int col) const noexcept
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
    const std::vector<float> m_data;
    const unsigned m_width;
    const unsigned m_height;
};

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("testing PixelWrapper accessor")
{
    PixelWrapper pw ({1, 2, 3, 4, 5, 6, 7, 8, 9}, 3, 3);
    CHECK_EQ(pw.get(0, 0), 1);
    CHECK_EQ(pw.get(0, -1), 1);
    CHECK_EQ(pw.get(-1, 0), 1);
    CHECK_EQ(pw.get(1, -1), 4);
    CHECK_EQ(pw.get(1, 3), 6);
    CHECK_EQ(pw.get(3, 3), 9);
}
#endif


float windowMean(const PixelWrapper& pixels, int cx, int cy)
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
    PixelWrapper pw ({77,63,31,29,8,17,72,9,92,43,8,57,83,35,78,71,59,38,39,43,42,22,50,4,56,5,87,86,34,97,95,99,16,0,25,35,23,76,23,45,26,35,90,1,13,39,84,21,94,97,38,98,12,76,58,62,49,22,14,64,80,67,47,94,59,23,68,32,75,100,27,93,70,10,25,93,48,88,78,2,77}, 9, 9);
    CHECK(windowMean(pw, 4, 4) == doctest::Approx(50.8765).epsilon(0.0001));
    CHECK(windowMean(pw, 0, 4) == doctest::Approx(54.4691).epsilon(0.0001));
}
#endif


float windowStd(const PixelWrapper &pixels, int cx, int cy)
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
    PixelWrapper pw ({77,63,31,29,8,17,72,9,92,43,8,57,83,35,78,71,59,38,39,43,42,22,50,4,56,5,87,86,34,97,95,99,16,0,25,35,23,76,23,45,26,35,90,1,13,39,84,21,94,97,38,98,12,76,58,62,49,22,14,64,80,67,47,94,59,23,68,32,75,100,27,93,70,10,25,93,48,88,78,2,77}, 9, 9);
    CHECK(windowStd(pw, 4, 4, 0) == doctest::Approx(271.6298).epsilon(0.0001));
}
#endif


float calcZncc(const PixelWrapper& pixL, const PixelWrapper& pixR, int cx, int cy, int d)
{
    const float meanL = windowMean(pixL, cx, cy); // TODO cache this value
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


//int findBestDisparity2(const PixelWrapper& pixL, const PixelWrapper& pixR, int cx, int cy)
//{
//    const int D = WINDOW / 2;
//
//    const float meanL = windowMean(pixL, cx, cy);
//    float bestZncc = 0.0f;
//    int bestd = 0;
//
//    for (int d = 0; d < MAX_D; ++d) {
//        const float meanR = windowMean(pixR, cx - d, cy);
//
//        float upsum = 0.0f, downSumL = 0.0f, downSumR = 0.0f;
//
//        for (int row = cy - D; row <= cy + D; ++row) {
//            for (int col = cx - D; col <= cx + D; ++col) {
//                upsum += (pixL.get(row, col) - meanL) * (pixR.get(row, col - d) - meanR);
//            }
//        }
//        for (int row = cy - D; row <= cy + D; ++row) {
//            for (int col = cx - D; col <= cx + D; ++col) {
//                const float val = pixL.get(row, col) - meanL;
//                downSumL += val * val;
//            }
//        }
//        for (int row = cy - D; row <= cy + D; ++row) {
//            for (int col = cx - D; col <= cx + D; ++col) {
//                const float val = pixR.get(row, col - d) - meanR;
//                downSumR += val * val;
//            }
//        }
//        const float zncc = upsum / sqrtf(downSumL) / sqrtf(downSumR);
//        if (zncc > bestZncc) {
//            bestZncc = zncc;
//            bestd = d;
//        }
//    }
//    return bestd;
//}


int findBestDisparity(const PixelWrapper& pixL, const PixelWrapper& pixR, int cx, int cy)
{
    float best_zncc = 0.0f;
    int best_disp = 0;
    for (int disp = 0; disp < MAX_D; ++disp) {
        const float zncc = calcZncc(pixL, pixR, cx, cy, disp);
        if (zncc > best_zncc) {
            best_zncc = zncc;
            best_disp = disp;
        }
    }
    return best_disp;
}

#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

int main()
{
    unsigned width, height;
    auto grey1 = loadGreyPixels("im0.png", width, height);
    auto grey2 = loadGreyPixels("im1.png", width, height);

    width /= 4;
    height /= 4;

    PixelWrapper greyPx1(std::move(grey1), width, height);
    PixelWrapper greyPx2(std::move(grey2), width, height);

    {
        std::vector<int> filtered(width * height);
        unsigned index = 0;
        for (unsigned row = 0; row < height; ++row) {
            for (unsigned col = 0; col < width; ++col) {
                filtered[index++] = findBestDisparity(greyPx1, greyPx2, col, row);
            }
            static int lastProgress = -1;
            const auto currentProgress = static_cast<int>(static_cast<float>(row) / height * 100.f);
            if (lastProgress != currentProgress) {
                std::cout << "progress: " << currentProgress << " %" << std::endl;
                lastProgress = currentProgress;
            }
        }
        std::cout << "max in filtered: " << *std::max_element(filtered.cbegin(), filtered.cend()) << std::endl;
        savePixels(filtered, width, height);
    }

    return 0;
}

#endif