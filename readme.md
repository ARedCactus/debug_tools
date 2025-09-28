# Example

## 纯C++
- cd 到cmakelists.txt 目录
mkdir build
cd build
cmake ..
make
sudo make install
./example

## ROS2
colcon build --packages-select debug_tools
source install/setup.bash
ros2 run debug_tools example

# Using in other package
- 修改cmakelists.txt
find_package(debug_tools REQUIRED)
target_compile_definitions(example PRIVATE DEBUG_LOG_PATH=\"${CMAKE_SOURCE_DIR}/log/\") #可选,手动指定日志的输出路径
target_link_libraries(your_executable debug_tools::debug_tools)



