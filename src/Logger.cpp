#include "Logger.hpp"
#include <iostream>
#include "../thirdparty/lodepng.h"


int Logger::m_lastBars = 0;


void Logger::logLoad(unsigned code, const char *filename) {
    if (code) {
        std::cout << "decoder error " << code << ": " << lodepng_error_text(code) << std::endl;
    } else {
        std::cout << "loading '" << filename << "' was successful" << std::endl;
    }
}


void Logger::logSave(unsigned code, const char *filename) {
    if (code) {
        std::cout << "encoder error " << code << ": " << lodepng_error_text(code) << std::endl;
    } else {
        std::cout << "successfully saved file: " << filename << std::endl;
    }
}


void Logger::logProgress(const char *text, float percent) {
    const int BARS = 40;

    const auto currentBars = static_cast<int>(percent * BARS);
    if (currentBars == m_lastBars) {
        return;
    }
    m_lastBars = currentBars;
    std::cout << text << "\t[";
    for (int i = 0; i < BARS; ++i) {
        if (i <= currentBars) {
            std::cout << "=";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "]\r";
    std::cout.flush();
}


void Logger::endProgress(const char *text) {
    m_lastBars = 0;
    std::cout << text << std::endl;
}
