#include "Logger.hpp"
#include <iostream>
#include "../thirdparty/lodepng.h"


void Logger::logLoad(unsigned code, const char *filename)
{
    if (code) {
        std::cout << "decoder error " << code << ": " << lodepng_error_text(code) << std::endl;
    } else {
        std::cout << "loading '" << filename << "' was successful" << std::endl;
    }
}


void Logger::logSave(unsigned code, const char *filename)
{
    if (code) {
        std::cout << "encoder error " << code << ": " << lodepng_error_text(code) << std::endl;
    } else {
        std::cout << "successfully saved file: " << filename << std::endl;
    }
}


void Logger::logProgress(const char *text, float percent)
{
    // TODO implement
}
