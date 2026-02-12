cmake_minimum_required(VERSION 3.8)
project(amigatw)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
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

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

add_executable(amiga_service src/amiga_service.cpp)
ament_target_dependencies(amiga_service rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros std_msgs motionservice)

add_executable(service_control src/service_control.cpp)
ament_target_dependencies(service_control rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros std_msgs motionservice)

add_executable(amiga_control src/amiga_main.cpp)
ament_target_dependencies(amiga_control rclcpp)

add_executable(forward_control src/forward_control.cpp)
ament_target_dependencies(forward_control rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros)

add_executable(backward_control src/backward_control.cpp)
ament_target_dependencies(backward_control rclcpp geometry_msgs sensor_msgs nav_msgs tf2_ros)

add_executable(stopper src/stopper.cpp)
ament_target_dependencies(stopper rclcpp geometry_msgs nav_msgs tf2_ros)

# Install
install(TARGETS 
  amiga_control
  forward_control
  backward_control
  stopper
  amiga_service
  service_control
  DESTINATION lib/${PROJECT_NAME})

install(PROGRAMS
  scripts/visualizer.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
