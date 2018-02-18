//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>
#include <memory>
#include <algorithm>
#include <Logger.hpp>
#include "PixelUtils.hpp"

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../thirdparty/doctest.h"
#endif


constexpr int WINDOW = 9;
constexpr int MAX_D = 260 / 4;
constexpr int CROSS_TH = 8;


float windowMean(const Pixelsf& pixels, int cx, int cy) {
    float sum = 0.0f;
    pixels.enumerateWindow(cx, cy, WINDOW, [&sum](float value){
        sum += value;
    });
    return sum / static_cast<float>(WINDOW * WINDOW);
}

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("check if windowMean is correct") {
    Pixelsf pw ({77,63,31,29,8,17,72,9,92,43,8,57,83,35,78,71,59,38,39,43,42,22,50,4,56,5,87,86,34,97,95,99,16,0,25,35,23,76,23,45,26,35,90,1,13,39,84,21,94,97,38,98,12,76,58,62,49,22,14,64,80,67,47,94,59,23,68,32,75,100,27,93,70,10,25,93,48,88,78,2,77}, 9, 9);
    CHECK(windowMean(pw, 4, 4) == doctest::Approx(50.8765).epsilon(0.0001));
    CHECK(windowMean(pw, 0, 4) == doctest::Approx(54.4691).epsilon(0.0001));
}
#endif


float windowStd(const Pixelsf &pixels, int cx, int cy) {
    const float mean = windowMean(pixels, cx, cy);
    float sum = 0.0f;
    pixels.enumerateWindow(cx, cy, WINDOW, [&sum, mean](float value) {
        sum += (value - mean)*(value - mean);
    });
    return sqrtf(sum);
}

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("check if windowStd is correct") {
    Pixelsf pw ({77,63,31,29,8,17,72,9,92,43,8,57,83,35,78,71,59,38,39,43,42,22,50,4,56,5,87,86,34,97,95,99,16,0,25,35,23,76,23,45,26,35,90,1,13,39,84,21,94,97,38,98,12,76,58,62,49,22,14,64,80,67,47,94,59,23,68,32,75,100,27,93,70,10,25,93,48,88,78,2,77}, 9, 9);
    CHECK(windowStd(pw, 4, 4) == doctest::Approx(271.6298).epsilon(0.0001));
}
#endif


float calcZncc(const Pixelsf& pixL, const Pixelsf& pixR, int cx, int cy, int d, float meanL) {
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


int findBestDisparity(const Pixelsf& pixL, const Pixelsf& pixR, int cx, int cy, bool invertD) {
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


Pixelsi calcDepthMap(const Pixelsf& leftPixels, const Pixelsf& rightPixels, bool invertD) {
    Logger::startProgress("calculating depth map");
    auto depthmap = pixelZip<float, int>(leftPixels, rightPixels,
                                         [invertD, &leftPixels, &rightPixels](int row, int col) {
                                             return findBestDisparity(leftPixels, rightPixels, col, row, invertD);
                                         });
    Logger::endProgress();
    return depthmap;
}


Pixelsi normalize(const Pixelsi& input) {
    std::vector<int> normalizedData(input.getWidth() * input.getHeight());
    for (int i = 0; i < input.getData().size(); ++i) {
        normalizedData[i] = input.getData()[i] * 255 / (MAX_D - 1);
    }
    return Pixelsi(std::move(normalizedData), input.getWidth(), input.getHeight());
}


Pixelsi crossCheck(const Pixelsi& in1, const Pixelsi& in2) {
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
    Logger::startProgress("calculating occlusion fill");
    const int MAX_OFFSET = 50;

    auto fillFun = [&in](int row, int col, int offset, int& result) {
        for (int r = row - offset; r < row + offset; ++r) {
            for (int c = col - offset; c < col + offset; ++c) {
                if (in.get(r, c) != 0) {
                    result = in.get(r, c);
                    return true;
                }
            }
        }
        return false;
    };

    std::vector<int> result(in.getData().size());
    int index = 0;
    for (int row = 0; row < in.getHeight(); ++row) {
        for (int col = 0; col < in.getWidth(); ++col) {
            int newData = in.get(row, col);
            if (newData == 0) {
                for (int offset = 1; offset <= MAX_OFFSET; ++offset) {
                    if (fillFun(row, col, offset, newData)) {
                        break;
                    }
                }
            }
            result[index++] = newData;
        }
        Logger::logProgress(1.f * row / in.getHeight());
    }
    Logger::endProgress();

    return Pixelsi(std::move(result), in.getWidth(), in.getHeight());
}


#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

int main() {
    auto greyPx1 = PixelUtils::loadGrey("im0.png");
    auto greyPx2 = PixelUtils::loadGrey("im1.png");

    auto depth1 = calcDepthMap(greyPx1, greyPx2, false);
    auto depth2 = calcDepthMap(greyPx2, greyPx1, true);

    auto crossChecked = normalize(crossCheck(depth1, depth2));
    PixelUtils::save(crossChecked, "depthmap.png");
    PixelUtils::save(occlusionFill(crossChecked), "occluded.png");

    return 0;
}

#endif