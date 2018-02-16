#ifndef DISPARITY_CPU_PIXELUTILS_HPP
#define DISPARITY_CPU_PIXELUTILS_HPP


#include "Pixels.hpp"


namespace PixelUtils {

Pixelsf loadGrey    (const char* filename);
Pixelsi load        (const char* filename);
void    save        (const Pixelsi &pixels, const char* filename);

};


#endif //DISPARITY_CPU_PIXELUTILS_HPP
