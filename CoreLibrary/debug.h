#ifndef DEBUG_H
#define DEBUG_H
#include <mutex>
#include <iostream>

static std::mutex s_debugSection;

/// Thread safe debug output
class DebugStream
{
public:
    inline DebugStream(const std::string area) {
        s_debugSection.lock();
        std::cout << "\033[1;34m" << area << "\033[1;37m>\033[1;32m";
    }

    ~DebugStream() {
        std::cout << "\033[0m" << std::endl;
        s_debugSection.unlock();
    }

    static std::string timestamp(const uint64_t t) {
        uint64_t us = t % 1000;
        uint64_t ms = t / 1000;
        uint64_t s = ms / 1000;
        ms = ms % 1000;

        std::string _s = std::to_string(s);
        _s += "s:";
        _s += std::to_string(ms);
        _s += "ms:";
        _s += std::to_string(us);
        _s += "us";

        return _s;
    }

    inline const DebugStream &operator<<(const std::string output) const { std::cout << " " << output ; return *this; }
    inline const DebugStream &operator<<(const uint64_t output) const { std::cout << " " << output; return *this; }
    inline const DebugStream &operator<<(const int64_t output) const { std::cout << " " << output; return *this; }
    inline const DebugStream &operator<<(const uint32_t output) const { std::cout << " " << output; return *this; }
    inline const DebugStream &operator<<(const uint16_t output) const { std::cout << " " << output; return *this; }
    inline const DebugStream &operator<<(const uint8_t output) const { std::cout << " " << (uint64_t)output; return *this; }
    inline const DebugStream &operator<<(const char *output) const { std::cout << " " << output; return *this; }
    inline const DebugStream &operator<<(const void *output) const { std::cout << std::hex <<  " 0x" <<  (uint64_t)output << std::dec; return *this; }
    inline const DebugStream &operator<<(const bool output) const { std::cout << (output ? " true" : " false"); return *this; }
};

static inline const DebugStream debug(const std::string area) { return DebugStream(area); }

#endif // DEBUG_H
