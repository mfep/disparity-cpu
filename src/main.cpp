//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "Logger.hpp"
#include "Disparity.hpp"
#include "PixelUtils.hpp"
#include "CliOptions.hpp"


#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

int main(int argc, const char* argv[]) {
    CliOptions::parse(argc, argv);
    Logger::logInit();

    using namespace DisparityAlgorithm;

    const auto greyPx1 = PixelUtils::loadGrey("im0.png");
    const auto greyPx2 = PixelUtils::loadGrey("im1.png");

    const auto depth1 = calcDepthMap(greyPx1, greyPx2, false);
    const auto depth2 = calcDepthMap(greyPx2, greyPx1, true);

    const auto crossChecked = normalize(crossCheck(depth1, depth2));
    PixelUtils::save(occlusionFill(crossChecked), "occluded.png");

    return 0;
}

#endif