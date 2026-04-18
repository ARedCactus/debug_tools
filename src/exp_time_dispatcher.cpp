#include "debug_tools/time_dispatcher.h"
#include <thread>
#include <iostream>

struct ImuMsg{
    uint64_t timestamp;
    float ax, ay, az;
};//struct ImuMsg

struct LidarMsg{
    uint64_t timestamp;
    int points;
};//struct LidarMsg

debug_tools::TimeDispatcher<1024, ImuMsg, LidarMsg> dispatcher_;
std::atomic<bool> running_{true};

inline uint64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

int count_imu{0};
void thdImu(){
    while(running_){
        ImuMsg msg{now_ns(), 0.0f, 0.0f, 0.0f};

        while(!dispatcher_.push(msg)){}
        count_imu++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int count_lidar{0};
void thdLidar(){
    while(running_){
        LidarMsg msg{now_ns(), 0};
        
        while(!dispatcher_.push(msg)){}
        count_lidar++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int count_out_imu{0}, count_out_lidar{0};
int main(){
    std::thread t1(thdImu), t2(thdLidar);
    std::thread consumer([](){
        while(running_){
            ImuMsg imu;
            if(dispatcher_.tryPop(imu)){
                std::cout << "[IMU]   " << imu.timestamp <<  "\n";
                count_out_imu++;
                continue;
            }  

            LidarMsg lidar;
            if(dispatcher_.tryPop(lidar)){
                std::cout << "[LIDAR] " << lidar.timestamp << "\n";
                count_out_lidar++;
                continue;
            } 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(5));
    running_ = false;

    t1.join();
    t2.join();
    consumer.join();

    std::cout << count_imu << ":" << count_out_imu << " " 
            << count_lidar << ":" << count_out_lidar << "\n";

    return 0;
}