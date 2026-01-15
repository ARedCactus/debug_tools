#ifndef DEBUG_TOOLS_TOOLS_H
#define DEBUG_TOOLS_TOOLS_H

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <sstream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

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

struct YMDData{
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
}; //struct YMDData

/**
 * @brief 独立线程写入文件，适配多线程环境
 */
class LogWriter{
private:
    std::queue<std::string> log_queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool is_running_{true};
    std::thread worker_;

public:
    LogWriter(){
#ifdef DEBUG_LOG_PATH
        std::string file_path = std::string(DEBUG_LOG_PATH) + "/" + YMDData().getDateString() + ".log";
        // std::cout << "[LogWriter is running] log path: " << file_path << "\n";
        ConsoleColor::print("[LogWriter is running]\n", Color::MAGENTA);
        ConsoleColor::print("Log path: ", Color::GREEN);
        ConsoleColor::print(file_path + "\n", Color::YELLOW);

        worker_ = std::thread([this, file_path](){
            std::ofstream ofs(file_path, std::ios::app);
            if(!ofs.is_open()){
                std::cerr << "[LogWriter] Failed to open file: " << file_path << "\n";
                return;
            }
            std::unique_lock<std::mutex> lock(mtx_);
            while(is_running_ || !log_queue_.empty()){
                cv_.wait(lock, [this]{ return !log_queue_.empty() || !is_running_; });
                while(!log_queue_.empty()){
                    ofs << log_queue_.front();
                    log_queue_.pop();
                    ofs.flush();
                }
            }
        });
#endif
    }

    static LogWriter& getInstance(){
        static LogWriter instance; // 第一次调用时构造
        return instance;
    }

    template<typename T> void log(const T& l){
#ifdef DEBUG_LOG_PATH
        std::ostringstream oss;
        oss << std::dec << l;
        std::string msg = oss.str();
        {   
            std::lock_guard<std::mutex> lock(mtx_);
            log_queue_.push(msg);
        }
        cv_.notify_one();
#endif
    }

    ~LogWriter(){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            is_running_ = false;
        }
        cv_.notify_all();
        if(worker_.joinable()) worker_.join();
    }
}; //class LogWriter

class Debug{
private:
    Color color_;
    LogWriter& log_ = LogWriter::getInstance();

    template <class T> T unpacker(const T& t){
        ConsoleColor::print(t, color_);
        std::cout << " ";
#ifdef DEBUG_LOG_PATH
        log_.log(t);
        log_.log(" ");
#endif
        return t;
    }

    void unpacker(const char* t){
        ConsoleColor::print(t, color_);
        std::cout << " ";
#ifdef DEBUG_LOG_PATH
        log_.log(t);
        log_.log(" ");
#endif
    }

public:
    Debug(Color color = Color::RESET){
        color_ = color;
    }

    virtual ~Debug() = default;

    template <typename T, typename... Args> void print(const T& t, const Args&... data){
        std::cout << std::dec;
        ConsoleColor::print(t, color_);
        std::cout << " ";
#ifdef DEBUG_LOG_PATH
        log_.log(t);
        log_.log(" ");
#endif
        (unpacker(data), ...);
        std::cout << "\n";
#ifdef DEBUG_LOG_PATH
        log_.log("\n");
#endif
    }

    template <typename T> void print(const T& t){
        std::cout << std::dec;
        ConsoleColor::print(t, color_);
        std::cout << "\n";
#ifdef DEBUG_LOG_PATH
        log_.log(t);
        log_.log("\n");
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

    template<typename T>
    inline T toTimeStamp(int32_t sec, u_int32_t nanosec){
        if constexpr (std::is_same_v<T, u_int64_t>)
            return static_cast<uint64_t>(sec) * 1000000000ULL + static_cast<uint64_t>(nanosec);
        else if constexpr (std::is_same_v<T, double>)
            return static_cast<double>(sec) + static_cast<double>(nanosec) * 1e-9;
        else return static_cast<T>(0);
    }

    inline double toTimeStamp(uint64_t ts_ns){
        return static_cast<double>(ts_ns) * 1e-9;
    }
}; //class Timer
} //namespace debug_tools

#endif //DEBUG_TOOLS_TOOLS_H