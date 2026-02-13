#ifndef AMIGA__LINEAR_CONTROLLER_HPP_
#define AMIGA__LINEAR_CONTROLLER_HPP_

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <memory>
#include <cmath>

namespace amigatw
{

class LinearController : public rclcpp::Node
{
public:
    // Constructor and destructor
    LinearController();
    explicit LinearController(const rclcpp::NodeOptions & options);
    virtual ~LinearController() = default;

    // Public methods
    void setLinearVelocity(double velocity);
    void setPIDGains(double kp, double ki, double kd);
    double getCurrentYaw() const;

private:
    // Callbacks
    void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg);
    void controlLoop();

    // Helper methods
    double computePIDCorrection(double current_error);
    void resetPID();

    // ROS components
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::TimerBase::SharedPtr control_timer_;
    
    // Control variables
    double current_yaw_;
    double target_linear_velocity_;
    
    // PID parameters
    double kp_, ki_, kd_;
    double integral_error_;
    double prev_error_;
    rclcpp::Time prev_time_;
};

}  // namespace amigatw

#endif  // AMIGA__LINEAR_CONTROLLER_HPP_

