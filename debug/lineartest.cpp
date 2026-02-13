#include "amigatw/linear_controller.hpp"
#include <rclcpp/rclcpp.hpp>
#include <thread>
#include <chrono>

class LinearControllerTester : public rclcpp::Node
{
public:
    LinearControllerTester()
    : Node("linear_controller_tester")
    {
        // Create an instance of the LinearController
        controller_ = std::make_shared<amigatw::LinearController>();
        
        // Configure it with test parameters
        controller_->setLinearVelocity(0.2);  // Slower speed for testing
        controller_->setPIDGains(1.0, 0.05, 0.2);  // Softer gains
        
        RCLCPP_INFO(get_logger(), "=== LINEAR CONTROLLER TEST ===");
        RCLCPP_INFO(get_logger(), "Created and configured LinearController");
        RCLCPP_INFO(get_logger(), "Linear velocity: 0.2 m/s");
        RCLCPP_INFO(get_logger(), "PID gains: Kp=1.0, Ki=0.05, Kd=0.2");
        
        // Create a timer to periodically report status
        report_timer_ = create_wall_timer(
            std::chrono::seconds(2),
            std::bind(&LinearControllerTester::reportStatus, this));
        
        // FIXED: Add the node's base interface, not the controller directly
        executor_.add_node(controller_->get_node_base_interface());
        
        // Start spinner thread
        spinner_thread_ = std::thread([this]() { executor_.spin(); });
        
        RCLCPP_INFO(get_logger(), "Test node spinning. Press Ctrl+C to stop.");
    }
    
    ~LinearControllerTester()
    {
        executor_.cancel();
        if (spinner_thread_.joinable()) {
            spinner_thread_.join();
        }
    }
    
private:
    void reportStatus()
    {
        double current_yaw = controller_->getCurrentYaw();
        RCLCPP_INFO(get_logger(), "Status - Current yaw: %.3f rad (%.1f deg)", 
                   current_yaw, current_yaw * 180.0 / M_PI);
        
        // Test dynamic reconfiguration
        static int counter = 0;
        counter++;
        
        if (counter == 5) {
            controller_->setLinearVelocity(0.3);
            RCLCPP_INFO(get_logger(), "Changed velocity to 0.3 m/s");
        }
        if (counter == 10) {
            controller_->setPIDGains(2.0, 0.1, 0.5);
            RCLCPP_INFO(get_logger(), "Changed PID gains to aggressive mode");
        }
    }
    
    std::shared_ptr<amigatw::LinearController> controller_;
    rclcpp::executors::SingleThreadedExecutor executor_;
    std::thread spinner_thread_;
    rclcpp::TimerBase::SharedPtr report_timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    
    auto tester = std::make_shared<LinearControllerTester>();
    rclcpp::spin(tester);
    
    rclcpp::shutdown();
    return 0;
}
