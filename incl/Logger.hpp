#ifndef CPU_GOLDEN_LOGGER_HPP
#define CPU_GOLDEN_LOGGER_HPP


namespace Logger {

void logLoad        (unsigned code, const char *filename);
void logSave(unsigned code, const char *filename);
void logProgress    (const char* text, float percent);

};


#endif //CPU_GOLDEN_LOGGER_HPP
