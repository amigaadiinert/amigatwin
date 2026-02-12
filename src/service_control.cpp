#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <motionservice/srv/mode_control.hpp>
#include <memory>
#include <cmath>
#include <string>

class AmigaController : public rclcpp::Node
{
public:
    AmigaController() : Node("amiga_controller"), current_mode_("stop")
    {
        // Publishers
        pub_ = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        
        // Subscribers
        imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
            "/imu", 10, std::bind(&AmigaController::imuCallback, this, std::placeholders::_1));
        
        // Service
        service_ = create_service<motionservice::srv::ModeControl>(
            "set_mode",
            std::bind(&AmigaController::handleModeChange, this,
                     std::placeholders::_1, std::placeholders::_2));
        
        // Timer
        control_timer_ = create_wall_timer(
            std::chrono::milliseconds(100),
            [this](){ controlLoop(); });
        
        // Initialize PID
        initializePID();
        
        RCLCPP_INFO(get_logger(), "Amiga Controller started. Current mode: %s", current_mode_.c_str());
    }

private:
    void initializePID()
    {
        kp_ = 2.0;
        ki_ = 0.1;
        kd_ = 0.5;
        integral_error_ = 0.0;
        prev_error_ = 0.0;
        prev_time_ = this->now();
        current_yaw_ = 0.0;
    }
    
    void handleModeChange(
        const std::shared_ptr<motionservice::srv::ModeControl::Request> request,
        std::shared_ptr<motionservice::srv::ModeControl::Response> response)
    {
        std::string new_mode = request->mode;
        
        if (new_mode == "forward" || new_mode == "backward" || new_mode == "stop") {
            current_mode_ = new_mode;
            
            // Reset PID when changing modes
            if (new_mode == "stop") {
                integral_error_ = 0.0;
                prev_error_ = 0.0;
                prev_time_ = this->now();
            }
            
            response->success = true;
            response->message = "Mode changed to: " + new_mode;
            RCLCPP_INFO(get_logger(), "Mode changed to: %s", new_mode.c_str());
        } else {
            response->success = false;
            response->message = "Invalid mode. Use 'forward', 'backward', or 'stop'";
            RCLCPP_WARN(get_logger(), "Invalid mode requested: %s", new_mode.c_str());
        }
    }
    
    void imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
        double x = msg->orientation.x;
        double y = msg->orientation.y;
        double z = msg->orientation.z;
        double w = msg->orientation.w;
        
        double siny_cosp = 2.0 * (w * z + x * y);
        double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
        current_yaw_ = std::atan2(siny_cosp, cosy_cosp);
    }
    
    double computePIDCorrection(double current_error)
    {
        auto now = this->now();
        double dt = (now - prev_time_).seconds();
        
        if (dt <= 0) dt = 0.1;
        
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

    void controlLoop()
    {
        auto twist = geometry_msgs::msg::Twist();
        
        // Set velocity based on mode
        if (current_mode_ == "forward") {
            twist.linear.x = 0.5;
        } else if (current_mode_ == "backward") {
            twist.linear.x = -0.5;
        } else { // stop
            twist.linear.x = 0.0;
            twist.angular.z = 0.0;
            pub_->publish(twist);
            return;
        }
        
        // PID correction
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

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Service<motionservice::srv::ModeControl>::SharedPtr service_;
    rclcpp::TimerBase::SharedPtr control_timer_;
    
    std::string current_mode_;
    double current_yaw_;
    double kp_, ki_, kd_;
    double integral_error_;
    double prev_error_;
    rclcpp::Time prev_time_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AmigaController>());
    rclcpp::shutdown();
    return 0;
}