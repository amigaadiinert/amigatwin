#include "amigatw/linear_controller.hpp"

namespace amigatw
{

LinearController::LinearController() 
: LinearController(rclcpp::NodeOptions()) 
{}

LinearController::LinearController(const rclcpp::NodeOptions & options)
: Node("linear_controller", options)
{
    // Publishers
    pub_ = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    
    // Subscribers
    imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
        "/imu", 10, std::bind(&LinearController::imuCallback, this, std::placeholders::_1));
    
    // Timers
    control_timer_ = create_wall_timer(
        std::chrono::milliseconds(100), [this](){ controlLoop(); });

    // Initialize PID controller
    kp_ = 2.0;
    ki_ = 0.1;
    kd_ = 0.5;
    
    resetPID();
    current_yaw_ = 0.0;
    target_linear_velocity_ = 0.5;
    
    RCLCPP_INFO(get_logger(), "Linear Controller started with IMU-based slip correction!");
}

void LinearController::imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
{
    double x = msg->orientation.x;
    double y = msg->orientation.y;
    double z = msg->orientation.z;
    double w = msg->orientation.w;
    
    double siny_cosp = 2.0 * (w * z + x * y);
    double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
    current_yaw_ = std::atan2(siny_cosp, cosy_cosp);
}

double LinearController::computePIDCorrection(double current_error)
{
    auto now = this->now();
    double dt = (now - prev_time_).seconds();
    
    if (dt <= 0) {
        dt = 0.1;
    }
    
    integral_error_ += current_error * dt;
    
    double max_integral = 1.0;
    if (integral_error_ > max_integral) integral_error_ = max_integral;
    if (integral_error_ < -max_integral) integral_error_ = -max_integral;
    
    double derivative = (current_error - prev_error_) / dt;
    double correction = kp_ * current_error + ki_ * integral_error_ + kd_ * derivative;
    
    double max_correction = 1.0;
    if (correction > max_correction) correction = max_correction;
    if (correction < -max_correction) correction = -max_correction;
    
    prev_error_ = current_error;
    prev_time_ = now;
    
    return correction;
}

void LinearController::controlLoop()
{
    auto twist = geometry_msgs::msg::Twist();
    twist.linear.x = target_linear_velocity_;
    
    double yaw_error = 0.0 - current_yaw_;
    double deadzone = 0.01;
    
    if (std::abs(yaw_error) > deadzone) {
        twist.angular.z = computePIDCorrection(yaw_error);
    } else {
        twist.angular.z = 0.0;
        integral_error_ = 0.0;
    }
    
    pub_->publish(twist);
}

void LinearController::setLinearVelocity(double velocity)
{
    target_linear_velocity_ = velocity;
}

void LinearController::setPIDGains(double kp, double ki, double kd)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    resetPID();
}

double LinearController::getCurrentYaw() const
{
    return current_yaw_;
}

void LinearController::resetPID()
{
    integral_error_ = 0.0;
    prev_error_ = 0.0;
    prev_time_ = this->now();
}

}  // namespace amigatw
