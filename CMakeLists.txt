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

# Define the library
add_library(linear_controller SHARED
  src/linear_controller.cpp
)

# IMPORTANT: Tell the compiler where to find header files during build
# This allows #include "amigatw/linear_controller.hpp" to work
target_include_directories(linear_controller PUBLIC
  # For building from source: includes the source directory's include folder
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  # For installed package: includes the install directory
  $<INSTALL_INTERFACE:include>
)

# Link dependencies to the library
ament_target_dependencies(linear_controller
  rclcpp
  geometry_msgs
  sensor_msgs
)

# ============================================
# TEST EXECUTABLE (uses the linear_controller library)
# ============================================

add_executable(testcode src/testcode.cpp)

# Tell testcode where to find the header files during build
target_include_directories(testcode PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the library to test executable
# This allows testcode to use ForwardController class
target_link_libraries(testcode linear_controller)

# Specify dependencies for testcode
ament_target_dependencies(testcode
  rclcpp
  geometry_msgs
  sensor_msgs
)

# ============================================
# OTHER EXECUTABLES
# ============================================

add_executable(amiga_service src/amiga_service.cpp)
ament_target_dependencies(amiga_service 
  rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros std_msgs motionservice
)

add_executable(service_control src/service_control.cpp)
ament_target_dependencies(service_control 
  rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros std_msgs motionservice
)

add_executable(amiga_control src/amiga_main.cpp)
ament_target_dependencies(amiga_control rclcpp)

add_executable(forward_control src/forward_control.cpp)
ament_target_dependencies(forward_control 
  rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros
)

add_executable(backward_control src/backward_control.cpp)
ament_target_dependencies(backward_control 
  rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros
)

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

# Install the linear_controller library
install(TARGETS linear_controller
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install header files so other packages can use this library
# This makes #include <amigatw/linear_controller.hpp> work in other packages
install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install all executables
install(TARGETS 
  amiga_control
  forward_control
  backward_control
  stopper
  amiga_service
  service_control
  testcode
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
