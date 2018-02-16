#ifndef CPU_GOLDEN_LOGGER_HPP
#define CPU_GOLDEN_LOGGER_HPP


class Logger {
public:
    static void logLoad        (unsigned code, const char *filename);
    static void logSave        (unsigned code, const char *filename);
    static void logProgress    (const char* text, float percent);
    static void endProgress    (const char* text);

private:
    static int m_lastBars;
};


#endif //CPU_GOLDEN_LOGGER_HPP
