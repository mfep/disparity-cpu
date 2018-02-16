#ifndef CPU_GOLDEN_PIXELUTILS_HPP
#define CPU_GOLDEN_PIXELUTILS_HPP


#include "Pixels.hpp"


namespace PixelUtils {

Pixelsf loadGrey    (const char* filename);
Pixelsi load        (const char* filename);
void    save        (const Pixelsi &pixels, const char* filename);

};


#endif //CPU_GOLDEN_PIXELUTILS_HPP
