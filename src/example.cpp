#include <chrono>
#include <thread>
#include "debug_tools/debug_tools.h"

int main(){
    //debug_tools::Debug().print() 可选颜色打印, 支持任意个参数
    //以文件方式记录运行日志 该功能默认关闭
    //开启方式：
    //方式1: cmakelists 中指定 DEBUG_LOG_PATH 的值 见cmakelists第49行
    //方式2： 手动宏定义 例如: #define DEBUG_LOG_PATH /home/ld/code/log/ 注意必须是绝对路径
    //该示例程序的日志输出在 log/目录下，*.log 文件命名为当天的时间
    debug_tools::Debug().print("default:", 123, std::string("debug"), "tools", 3.14);  //默认无颜色
    debug_tools::Debug(debug_tools::Color::MAGENTA).print("magenta:", 123, std::string("debug"), "tools", 3.14);
    debug_tools::Debug(debug_tools::Color::YELLOW).print("yellow:", 123, std::string("debug"), "tools", 3.14);
    debug_tools::Debug(debug_tools::Color::GREEN).print("green:", 123, std::string("debug"), "tools", 3.14);
    debug_tools::Debug(debug_tools::Color::RED).print("red:", 123, std::string("debug"), "tools", 3.14);
    debug_tools::Debug(debug_tools::Color::BLUE).print("blue:", 123, std::string("debug"), "tools", 3.14);

    debug_tools::Timer t_begin; //计时器,记录程序运行时间并在终端打印

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));  //休眠1000ms

    debug_tools::Timer t_middle(t_begin); t_middle.log("t_begin ~ t_middle sleep");

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));  //休眠2000ms

    debug_tools::Timer t_end(t_middle); t_end.log("t_middle ~ t_end sleep");

    debug_tools::Timer t_total(t_begin); t_total.log("t_begin ~ t_end sleep");

    return 0;
}