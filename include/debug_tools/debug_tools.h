#ifndef DEBUG_TOOLS_DEBUG_TOOLS_H
#define DEBUG_TOOLS_DEBUG_TOOLS_H

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <sstream>

// #define DEBUG_LOG_PATH
namespace debug_tools{

enum class Color {
    RESET   = 0,
    RED     = 1,
    GREEN   = 2,
    YELLOW  = 3,
    BLUE    = 4,
    MAGENTA = 5,
    CYAN    = 6,
    WHITE   = 7
};

class ConsoleColor{
public:
    template<typename T>  static void print(const T& text, Color color){
    #if defined(_WIN32) || defined(_WIN64)
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        WORD saved_attributes;

        // 保存当前属性
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        saved_attributes = consoleInfo.wAttributes;

        // 设置新颜色
        SetConsoleTextAttribute(hConsole, mapColor(color));
        std::cout << text;

        // 恢复原颜色
        SetConsoleTextAttribute(hConsole, saved_attributes);
    #else
        std::cout << "\033[" << mapColor(color) << "m" << text << "\033[0m";
    #endif
    }

private:
#if defined(_WIN32) || defined(_WIN64)
    static WORD mapColor(Color color){
        switch (color){
            case Color::RED:     return FOREGROUND_RED | FOREGROUND_INTENSITY;
            case Color::GREEN:   return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            case Color::YELLOW:  return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            case Color::BLUE:    return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case Color::MAGENTA: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case Color::CYAN:    return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case Color::WHITE:   return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            default:             return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    }
#else
    static int mapColor(Color color){
        switch (color){
            case Color::RED:     return 31;
            case Color::GREEN:   return 32;
            case Color::YELLOW:  return 33;
            case Color::BLUE:    return 34;
            case Color::MAGENTA: return 35;
            case Color::CYAN:    return 36;
            case Color::WHITE:   return 37;
            default:             return 0;
        }
    }
#endif
}; //class ConsoleColor

class Debug{
private:
    std::ofstream logfile_;
    Color color_;

    // 获取当天日期字符串
    std::string getDateString(){
        auto t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);   // Windows 安全版本
#else
        localtime_r(&t, &tm);   // Linux/macOS
#endif
        std::ostringstream oss;
        oss << (tm.tm_year + 1900) << "." << (tm.tm_mon + 1) << "." << tm.tm_mday;
        return oss.str();
    }

    template <class T> T unpacker(const T& t){
        // std::cout << t << " ";
        ConsoleColor::print(t, color_);
        std::cout << " ";
#ifdef DEBUG_LOG_PATH
        if(logfile_.is_open()) logfile_ << t << " ";
#endif
        return t;
    }

    void unpacker(const char* t){
        // std::cout << t << " ";
        ConsoleColor::print(t, color_);
        std::cout << " ";
#ifdef DEBUG_LOG_PATH
        if(logfile_.is_open()) logfile_ << t << " ";
#endif
    }

public:
    Debug(Color color = Color::RESET){
        color_ = color;
#ifdef DEBUG_LOG_PATH
        std::string log_path = std::string(DEBUG_LOG_PATH) + "/" + getDateString() + ".log";
        logfile_.open(log_path, std::ios::app);
        if(!logfile_){
            std::cerr << "[Debug Error] Failed to open log file: " << log_path << std::endl;
        }
#endif
    }

    ~Debug(){
#ifdef DEBUG_LOG_PATH
        if(logfile_.is_open()) logfile_.close();
#endif
    }

    template <typename T, typename... Args> void print(const T& t, const Args&... data){
        // std::cout << std::dec << t << " ";
        std::cout << std::dec;
        ConsoleColor::print(t, color_);
        std::cout << " ";
#ifdef DEBUG_LOG_PATH
        if(logfile_.is_open()) logfile_ << std::dec << t << " ";
#endif
        (unpacker(data), ...);
        std::cout << "\n";
#ifdef DEBUG_LOG_PATH
        if(logfile_.is_open()) logfile_ << "\n";
#endif
    }

    template <typename T> void print(const T& t){
        // std::cout << std::dec << t << "\n";
        std::cout << std::dec;
        ConsoleColor::print(t, color_);
        std::cout << "\n";
#ifdef DEBUG_LOG_PATH
        if(logfile_.is_open()) logfile_ << t << "\n";
#endif
    }
}; //class Debug


class Timer{
private:
    std::chrono::_V2::system_clock::time_point time_init_;
    int64_t duration_{0};

public:
    typedef std::shared_ptr<Timer> Ptr;

    Timer(){
        time_init_ = std::chrono::system_clock::now();
    }

    Timer(Timer& timer){
        time_init_ = std::chrono::system_clock::now();
        duration_ = std::chrono::duration_cast<std::chrono::milliseconds>(time_init_ - timer.time_init_).count();
    }

    void log(std::string log_a = "", std::string log_b = "ms"){
        std::cout << log_a << ": " << duration_ << log_b << "\n";
    }

    int64_t duration(){
        return duration_;
    }
}; //class Timer
} //namespace debug_tools

#endif