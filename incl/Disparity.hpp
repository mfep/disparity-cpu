#ifndef DISPARITY_CPU_DISPARITY_HPP
#define DISPARITY_CPU_DISPARITY_HPP

#include "Pixels.hpp"

Pixelsi calcDepthMap(const Pixelsf& leftPixels, const Pixelsf& rightPixels, bool invertD);
Pixelsi normalize(const Pixelsi& input);
Pixelsi crossCheck(const Pixelsi& in1, const Pixelsi& in2);
Pixelsi occlusionFill(const Pixelsi& in);

#endif //DISPARITY_CPU_DISPARITY_HPP
