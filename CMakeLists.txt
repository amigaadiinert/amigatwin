cmake_minimum_required(VERSION 3.8)
project(amigatw)

# Compiler options
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(motionservice REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# ============================================
# LINEAR CONTROLLER LIBRARY
# ============================================

add_library(linear_controller SHARED
  src/linear_controller.cpp
)

target_include_directories(linear_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(linear_controller
  rclcpp
  geometry_msgs
  sensor_msgs
)

# ============================================
# KEY LISTENER LIBRARY
# ============================================

add_library(key_listener SHARED
  src/key_listener.cpp
)

target_include_directories(key_listener PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(key_listener
  rclcpp
  std_msgs
)

# ============================================
# DEBUG/TEST EXECUTABLES (from debug folder)
# ============================================

# Forward Controller Test
add_executable(linear_controller_test debug/lineartest.cpp)

target_include_directories(linear_controller_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(linear_controller_test 
  linear_controller
)

ament_target_dependencies(linear_controller_test
  rclcpp
  geometry_msgs
  sensor_msgs
)

# Key Command Node (Main from debug)
add_executable(key_command_test debug/keytest.cpp)

target_include_directories(key_command_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(key_command_test
  key_listener
)

ament_target_dependencies(key_command_test
  rclcpp
  std_msgs
)
# ============================================
# INTEGRATION TEST EXECUTABLES
# ============================================

# Controller Integration Test
add_executable(controller_integration_test debug/integrationtest.cpp)

target_include_directories(controller_integration_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(controller_integration_test
  key_listener
  linear_controller
)

ament_target_dependencies(controller_integration_test
  rclcpp
  geometry_msgs
  sensor_msgs
  std_msgs
)
# ============================================
# OTHER EXECUTABLES (from src)
# ============================================

add_executable(stopper src/stopper.cpp)
ament_target_dependencies(stopper 
  rclcpp geometry_msgs nav_msgs tf2_ros
)

# ============================================
# INSTALLATION RULES
# ============================================

# Install launch files
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# Install libraries
install(TARGETS 
  linear_controller
  key_listener
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install header files
install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install all executables
install(TARGETS 
  stopper
  linear_controller_test
  key_command_test
  controller_integration_test
  DESTINATION lib/${PROJECT_NAME}
)

# Install Python scripts
install(PROGRAMS
  scripts/visualizer.py
  scripts/debug.py
  DESTINATION lib/${PROJECT_NAME}
)

# Standard ROS2 package generation
ament_package()
