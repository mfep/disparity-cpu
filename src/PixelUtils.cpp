#include <algorithm>
#include <sstream>
#include <memory>
#include <iostream>
#include "PixelUtils.hpp"
#include "../thirdparty/lodepng.h"


namespace {

Pixelsf preprocessPixels(const std::vector<unsigned char> &pixels, unsigned width, unsigned height) {
    const float
            R = 0.2126f,
            G = 0.7152f,
            B = 0.0722f;

    unsigned index = 0;
    std::vector<float> resized(pixels.size() / 16 / 3);
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
    return Pixelsf(std::move(resized), width, height);
}

}


Pixelsf PixelUtils::loadGrey(const char *filename)
{
    unsigned width, height;
    std::vector<unsigned char> pixels;
    unsigned error = lodepng::decode(pixels, width, height, filename, LCT_RGB);
    if (error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    } else {
		std::cout << "loading '" << filename << "' was successful" << std::endl;
	}
    return preprocessPixels(pixels, width, height);
}


void PixelUtils::save(const Pixelsi& pixels, const char* filename)
{
    std::vector<unsigned char> convertedPixels(pixels.getData().size());
    unsigned i = 0;
    for (auto f : pixels.getData()) {
        convertedPixels[i++] = static_cast<unsigned char>(f);
    }
    unsigned error = lodepng::encode(filename, convertedPixels, pixels.getWidth(), pixels.getHeight(), LCT_GREY, 8);
    if (error) {
        std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
}

