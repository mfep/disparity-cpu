//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "Disparity.hpp"
#include "PixelUtils.hpp"


#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

int main() {
    auto greyPx1 = PixelUtils::loadGrey("im0.png");
    auto greyPx2 = PixelUtils::loadGrey("im1.png");

    auto depth1 = calcDepthMap(greyPx1, greyPx2, false);
    auto depth2 = calcDepthMap(greyPx2, greyPx1, true);

    auto crossChecked = normalize(crossCheck(depth1, depth2));
    PixelUtils::save(occlusionFill(crossChecked), "occluded.png");

    return 0;
}

#endif