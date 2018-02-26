#ifndef DISPARITY_CPU_DISPARITY_HPP
#define DISPARITY_CPU_DISPARITY_HPP

#include "Pixels.hpp"

/// Provides functions to execute the disparity (ZNCC) algorithm and post-processing.
namespace DisparityAlgorithm {

/// Calculates the depth map from two input images.
/// \param leftPixels Left image data.
/// \param rightPixels Right image data.
/// \param invertD Should be true, if the order of left and right pixel data is reversed.
/// \return The depth map pixel data.
Pixelsi calcDepthMap(const Pixelsf &leftPixels, const Pixelsf &rightPixels, bool invertD);

/// Normalizes the output of the disparity algorithm. Ie. from 0-63 -> 0-255.
/// \param input Input pixel data.
/// \return Normalized pixel data.
Pixelsi normalize(const Pixelsi &input);

/// Runs cross-check post-processing algorithm.
/// If a difference between the pixel values in the inputs is greater than a threshold, that pixel's value
/// is going to be 0 in the output image.
/// \param in1 Input pixel data.
/// \param in2 Input pixel data.
/// \return CrossChecked output.
Pixelsi crossCheck(const Pixelsi &in1, const Pixelsi &in2);

/// Runs occlusion fill post-processing algorithm.
/// \param in Input pixel data.
/// \return Occlusion filled output data.
Pixelsi occlusionFill(const Pixelsi &in);

}   // namespace DisparityAlgorithm

#endif //DISPARITY_CPU_DISPARITY_HPP
