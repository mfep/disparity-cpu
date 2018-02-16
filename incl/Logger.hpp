#ifndef CPU_GOLDEN_LOGGER_HPP
#define CPU_GOLDEN_LOGGER_HPP


#include <chrono>


class Logger {
public:
    static void logLoad        (unsigned code, const char *filename);
    static void logSave        (unsigned code, const char *filename);
    static void startProgress  (const char* text);
    static void logProgress    (float percent);
    static void endProgress    ();

private:
    static int m_lastBars;
    static const char* m_progressText;
    static std::chrono::system_clock::time_point m_startTime;
};


#endif //CPU_GOLDEN_LOGGER_HPP
