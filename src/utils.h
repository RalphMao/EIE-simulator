
#ifndef UTILS
#define UTILS

#include<string>
#include<iostream>
#include<stdexcept>
#include<execinfo.h>
#include<stdlib.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define LOG(x) Message(__func__, __FILE__, __LINE__, (x))
#define LOG_DEBUG(x) if(DEBUG) LOG(x)

#define LOG_ERROR(x) MessageError(__func__, __FILE__, __LINE__, (x))

#define P_V(x) std::cout << #x ": " << x << std::endl

inline void Message(const char *func, const char *file, int line, std::string x) {
    std::cout << "LOG:      #call from " << func << "(): " << file << ':' << line << "#" << std::endl;
    std::cout << ">>> " << x << std::endl;
}

inline void MessageError(const char *func, const char *file, int line, std::string x) {
    std::cerr << "ERROR:       #call from " << func << "(): " << file << ':' << line << "#" <<  std::endl;
    std::cerr << ">>> " << x << std::endl;
    void *array[10];
    size_t size = backtrace(array,10);
    char **strings = backtrace_symbols(array, size);
    std::cerr << "##############Stack Trace#############" << std::endl;
    for (size_t i=0; i < size; i++) {
        std::cerr << strings[i] << std::endl;
    }
    free(strings);
    std::cerr << "##############Stack Trace#############" << std::endl;
    throw std::runtime_error("");
}
#endif
