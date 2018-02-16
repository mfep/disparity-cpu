//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include "PixelUtils.hpp"

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"
#endif


constexpr int WINDOW = 9;
constexpr int MAX_D = 260 / 4;
constexpr int CROSS_TH = 8;


float windowMean(const Pixelsf& pixels, int cx, int cy)
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


float windowStd(const Pixelsf &pixels, int cx, int cy)
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


float calcZncc(const Pixelsf& pixL, const Pixelsf& pixR, int cx, int cy, int d, float meanL)
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


int findBestDisparity(const Pixelsf& pixL, const Pixelsf& pixR, int cx, int cy, bool invertD)
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


Pixelsi calcDepthMap(const Pixelsf& leftPixels, const Pixelsf& rightPixels, bool invertD)
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

    return Pixelsi(std::move(depthMap), width, height);
}


Pixelsi normalize(const Pixelsi& input)
{
    std::vector<int> normalizedData(input.getWidth() * input.getHeight());
    for (int i = 0; i < input.getData().size(); ++i) {
        normalizedData[i] = input.getData()[i] * 255 / (MAX_D - 1);
    }
    return Pixelsi(std::move(normalizedData), input.getWidth(), input.getHeight());
}


Pixelsi crossCheck(const Pixelsi& in1, const Pixelsi& in2)
{
    std::vector<int> result(in1.getWidth() * in1.getHeight());
    for (int i = 0; i < in1.getData().size(); ++i) {
        const int px1 = in1.getData()[i];
        const int px2 = in2.getData()[i];
        if (abs(px1 - px2) > CROSS_TH) {
            result[i] = 0;
        } else {
            result[i] = (px1 + px2) / 2;
        }
    }
    return Pixelsi(std::move(result), in1.getWidth(), in1.getHeight());
}


Pixelsi occlusionFill(const Pixelsi& in)
{
    const int OCCLUSION_ITER = 5;

    std::vector<int> result(in.getWidth() * in.getHeight());
    unsigned index = 0;

    for (int row = 0; row < in.getHeight(); ++row) {
        for (int col = 0; col < in.getWidth(); ++col) {
            if (in.get(row, col) != 0) {
                result[index++] = in.get(row, col);
            } else {
                int occludedValue = 0;
                for (int occ_i = 1; occ_i <= OCCLUSION_ITER; ++occ_i) {
                    occludedValue = in.get(row + occ_i, col);
                    if (occludedValue != 0) break;
                    occludedValue = in.get(row - occ_i, col);
                    if (occludedValue != 0) break;
                    occludedValue = in.get(row, col + occ_i);
                    if (occludedValue != 0) break;
                    occludedValue = in.get(row, col - occ_i);
                    if (occludedValue != 0) break;
                }
                result[index++] = occludedValue;
            }
        }
    }

    return Pixelsi(std::move(result), in.getWidth(), in.getHeight());
}


#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

int main()
{
    auto greyPx1 = PixelUtils::loadGrey("im0.png");
    auto greyPx2 = PixelUtils::loadGrey("im1.png");

    auto depth1 = calcDepthMap(greyPx1, greyPx2, false);
    auto depth2 = calcDepthMap(greyPx2, greyPx1, true);

    PixelUtils::save(normalize(occlusionFill(crossCheck(depth1, depth2))), "depthmap.png");

    return 0;
}

#endif