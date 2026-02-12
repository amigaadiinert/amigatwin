#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <memory>
#include <cmath>

class Stopper : public rclcpp::Node
{
public:
    Stopper() : Node("stopper")
    {
        // Publishers
        pub_ = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        
        // Timers
        control_timer_ = create_wall_timer(
            std::chrono::milliseconds(100), [this](){ controlLoop(); });
        
        RCLCPP_INFO(get_logger(), "Controller has been stopped!");
    }

private:
    
    void controlLoop()
    {
        auto twist = geometry_msgs::msg::Twist();
        
        // Set linear velocity 
        twist.linear.x = 0.0;
        twist.angular.z = 0.0;
        
        // Publish the velocity command
        pub_->publish(twist);
    }

    // ROS components
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr control_timer_, direction_timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Stopper>());
    rclcpp::shutdown();
    return 0;
}