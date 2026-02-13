#include "key_listener/key_listener.hpp"
#include "amigatw/linear_controller.hpp"
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <memory>

namespace amigatw
{

class ControllerIntegrationNode : public rclcpp::Node
{
public:
    ControllerIntegrationNode()
    : Node("controller_integration_node")
    {
        // DO NOT create the controller here - it will be added to executor separately
        
        // Subscribe to key commands
        subscription_ = this->create_subscription<std_msgs::msg::Int32>(
            "/key_command", 10,
            std::bind(&ControllerIntegrationNode::commandCallback, this, std::placeholders::_1));
        
        RCLCPP_INFO(this->get_logger(), "Controller Integration Node started");
        RCLCPP_INFO(this->get_logger(), "  w -> Forward 0.5 m/s");
        RCLCPP_INFO(this->get_logger(), "  s -> Backward -0.5 m/s");
        RCLCPP_INFO(this->get_logger(), "  x -> Stop 0 m/s");
    }
    
    // Add a method to set the controller
    void setController(std::shared_ptr<amigatw::LinearController> controller)
    {
        controller_ = controller;
    }

private:
    void commandCallback(const std_msgs::msg::Int32::SharedPtr msg)
    {
        if (!controller_) {
            RCLCPP_WARN(this->get_logger(), "Controller not set!");
            return;
        }
        
        int command = msg->data;
        
        switch(command) {
            case 1:  // w - Forward
                controller_->setLinearVelocity(0.5);
                RCLCPP_INFO(this->get_logger(), "Command: Forward - Setting velocity to 0.5 m/s");
                break;
            case 2:  // s - Backward
                controller_->setLinearVelocity(-0.5);
                RCLCPP_INFO(this->get_logger(), "Command: Backward - Setting velocity to -0.5 m/s");
                break;
            case 0:  // x - Stop
                controller_->setLinearVelocity(0.0);
                RCLCPP_INFO(this->get_logger(), "Command: Stop - Setting velocity to 0.0 m/s");
                break;
            default:
                break;
        }
    }
    
    std::shared_ptr<amigatw::LinearController> controller_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr subscription_;
};

}  // namespace amigatw

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    
    // Create nodes
    auto key_listener = std::make_shared<key_listener::KeyListener>();
    auto linear_controller = std::make_shared<amigatw::LinearController>();
    auto integration_node = std::make_shared<amigatw::ControllerIntegrationNode>();
    
    // Set the controller in the integration node
    integration_node->setController(linear_controller);
    
    // Create executor and add ALL nodes
    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(key_listener);
    executor.add_node(linear_controller);  // IMPORTANT: Add the controller to executor!
    executor.add_node(integration_node);
    
    RCLCPP_INFO(rclcpp::get_logger("main"), "=== CONTROLLER INTEGRATION TEST ===");
    RCLCPP_INFO(rclcpp::get_logger("main"), "Press keys to control the robot:");
    RCLCPP_INFO(rclcpp::get_logger("main"), "  W - Forward (0.5 m/s)");
    RCLCPP_INFO(rclcpp::get_logger("main"), "  S - Backward (-0.5 m/s)");
    RCLCPP_INFO(rclcpp::get_logger("main"), "  X - Stop (0 m/s)");
    RCLCPP_INFO(rclcpp::get_logger("main"), "Press Ctrl+C to exit");
    
    // Spin
    executor.spin();
    
    rclcpp::shutdown();
    return 0;
}