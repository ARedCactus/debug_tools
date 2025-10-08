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
  add_executable(example src/example.cpp)
  target_compile_definitions(example PRIVATE DEBUG_LOG_PATH=\"${CMAKE_SOURCE_DIR}/log/\") #指定Log路径
  target_link_libraries(example PRIVATE ${PROJECT_NAME})

  # 纯C++安装可执行文件
  install(TARGETS example
    RUNTIME DESTINATION bin
  )
endif()

# ========== ROS2 兼容 ==========
# 如果有 package.xml，ROS2 会要求 ament_package
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/package.xml")
  find_package(ament_cmake REQUIRED)
  if(BUILD_EXAMPLES)
    install(TARGETS example
      DESTINATION lib/${PROJECT_NAME}
    )
  endif()
  ament_package()
endif()