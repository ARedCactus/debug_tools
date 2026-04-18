cmake_minimum_required(VERSION 3.8)
project(debug_tools LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -std=c++17)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

find_package(OpenCV REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# 安装头文件
install(DIRECTORY include/ DESTINATION include)

# 安装 INTERFACE target
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

# 生成 Config.cmake（支持 find_package）
include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION share/${PROJECT_NAME}/cmake
)

install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION share/${PROJECT_NAME}/cmake
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION share/${PROJECT_NAME}/cmake
)

# ========== 示例程序 ==========
option(BUILD_EXAMPLES "Build example programs" ON)

if(BUILD_EXAMPLES)
  add_executable(tools src/exp_tools.cpp)
  target_compile_definitions(tools PRIVATE DEBUG_LOG_PATH=\"${CMAKE_SOURCE_DIR}/log/\") #指定Log路径
  target_link_libraries(tools PRIVATE ${PROJECT_NAME} Threads::Threads)

  add_executable(mono_queue src/exp_mono_queue.cpp)
  target_compile_definitions(mono_queue PRIVATE PICTURE_PATH=\"${CMAKE_SOURCE_DIR}/cmake/\")
  target_link_libraries(mono_queue PRIVATE ${PROJECT_NAME} ${OpenCV_LIBS} Threads::Threads)

  add_executable(time_dispatcher src/exp_time_dispatcher.cpp)
  target_link_libraries(time_dispatcher PRIVATE ${PROJECT_NAME} Threads::Threads)
endif()

# ================= ROS2 =================
if(DEFINED ENV{AMENT_PREFIX_PATH})

  message(STATUS "Build in ROS2 (ament)")
  find_package(ament_cmake REQUIRED)
  install(TARGETS 
    tools
    mono_queue
    time_dispatcher
    RUNTIME DESTINATION bin
  )
  ament_package()

# ================= ROS1 =================
elseif(DEFINED ENV{CATKIN_DEVEL_PREFIX})

  message(STATUS "Build in ROS1 (catkin)")
  find_package(catkin REQUIRED)
  install(TARGETS 
    tools
    mono_queue
    time_dispatcher
    RUNTIME DESTINATION bin
  )
  catkin_package()

# ================= 非 ROS（Windows / Linux 纯 CMake） =================
else()
  message(STATUS "Build in pure CMake (no ROS)")
endif()