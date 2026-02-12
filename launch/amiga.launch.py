import os
from launch import LaunchDescription
from launch.actions import ExecuteProcess
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Update this path to your SDF file
    sdf_file = '/home/user/igngz/amiga.sdf'  
    
    # Launch Gazebo
    gazebo = ExecuteProcess(
        cmd=['ign', 'gazebo', '-r', sdf_file],
        output='screen'
    )
    
    # ROS2-Gazebo bridge
    bridge = ExecuteProcess(
        cmd=['ros2', 'run', 'ros_gz_bridge', 'parameter_bridge',
             '/lidar@sensor_msgs/msg/LaserScan[ignition.msgs.LaserScan',
             '/imu@sensor_msgs/msg/Imu[ignition.msgs.IMU',
             '/cmd_vel@geometry_msgs/msg/Twist]ignition.msgs.Twist'],
        output='screen'
    )
    
    return LaunchDescription([gazebo, bridge])
