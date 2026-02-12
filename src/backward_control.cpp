#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <memory>
#include <cmath>

class BackwordController : public rclcpp::Node
{
public:
    BackwordController() : Node("backword_controller")
    {
        // Publishers
        pub_ = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        
        // Subscribers
        imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
            "/imu", 10, std::bind(&BackwordController::imuCallback, this, std::placeholders::_1));
        
        // Timers
        control_timer_ = create_wall_timer(
            std::chrono::milliseconds(100), [this](){ controlLoop(); });

        // Initialize PID controller for angular correction
        // Kp: Proportional gain, Ki: Integral gain, Kd: Derivative gain
        kp_ = 2.0;   // Proportional gain - adjust based on response
        ki_ = 0.1;   // Integral gain - to eliminate steady-state error
        kd_ = 0.5;   // Derivative gain - to reduce overshoot
        
        integral_error_ = 0.0;
        prev_error_ = 0.0;
        prev_time_ = this->now();
        current_yaw_ = 0.0;
        
        RCLCPP_INFO(get_logger(), "Backword Controller started with IMU-based slip correction!");
    }

private:
    void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
        // Extract yaw from IMU quaternion (orientation around z-axis)
        // IMU orientation.z is the z component of the quaternion, not the yaw angle
        // We need to convert quaternion to Euler angles to get yaw
        
        // Get the quaternion components
        double x = msg->orientation.x;
        double y = msg->orientation.y;
        double z = msg->orientation.z;
        double w = msg->orientation.w;
        
        // Convert quaternion to yaw (rotation around z-axis)
        // Formula: yaw = atan2(2*(w*z + x*y), 1 - 2*(y*y + z*z))
        double siny_cosp = 2.0 * (w * z + x * y);
        double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
        current_yaw_ = std::atan2(siny_cosp, cosy_cosp);
        
        // For debugging: print the yaw angle
        // RCLCPP_DEBUG(get_logger(), "Current yaw: %.3f radians", current_yaw_);
    }
    
    double computePIDCorrection(double current_error)
    {
        auto now = this->now();
        double dt = (now - prev_time_).seconds();
        
        if (dt <= 0) {
            dt = 0.1; // Default to timer interval
        }
        
        // Calculate integral term (accumulated error)
        integral_error_ += current_error * dt;
        
        // Limit integral windup to prevent excessive correction
        double max_integral = 1.0; // Maximum integral accumulation
        if (integral_error_ > max_integral) integral_error_ = max_integral;
        if (integral_error_ < -max_integral) integral_error_ = -max_integral;
        
        // Calculate derivative term (rate of change of error)
        double derivative = (current_error - prev_error_) / dt;
        
        // PID formula: output = Kp*error + Ki*integral + Kd*derivative
        double correction = kp_ * current_error + ki_ * integral_error_ + kd_ * derivative;
        
        // Limit the maximum angular correction to prevent aggressive turns
        double max_correction = 1.0; // Maximum angular velocity in rad/s
        if (correction > max_correction) correction = max_correction;
        if (correction < -max_correction) correction = -max_correction;
        
        // Update previous values for next iteration
        prev_error_ = current_error;
        prev_time_ = now;
        
        return correction;
    }

    void controlLoop()
    {
        auto twist = geometry_msgs::msg::Twist();
        
        // Set linear velocity (Backword or backward)
        twist.linear.x = -0.5;
        
        // Calculate error: current yaw deviation from ideal (0 radians)
        // Positive error means turning right, negative means turning left
        double yaw_error = 0.0 - current_yaw_;
        
        // Small deadzone to prevent constant minor corrections
        double deadzone = 0.01; // 0.01 radians ≈ 0.57 degrees
        
        if (std::abs(yaw_error) > deadzone) {
            // Compute PID correction for angular velocity
            twist.angular.z = computePIDCorrection(yaw_error);
            
            // Log for debugging (uncomment for troubleshooting)
            // RCLCPP_DEBUG(get_logger(), "Yaw error: %.3f rad, Correction: %.3f rad/s", 
            //              yaw_error, twist.angular.z);
        } else {
            // No correction needed, maintain straight line
            twist.angular.z = 0.0;
            
            // Reset integral term when error is small to prevent windup
            integral_error_ = 0.0;
        }
        
        // Publish the velocity command
        pub_->publish(twist);
    }

    // ROS components
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::TimerBase::SharedPtr control_timer_, direction_timer_;
    
    // Control variables
    double current_yaw_;
    
    // PID parameters
    double kp_, ki_, kd_;
    double integral_error_;
    double prev_error_;
    rclcpp::Time prev_time_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BackwordController>());
    rclcpp::shutdown();
    return 0;
}