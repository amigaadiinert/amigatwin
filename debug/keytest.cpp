#include "key_listener/key_listener.hpp"
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <iostream>

class CommandPrinter : public rclcpp::Node
{
public:
    CommandPrinter()
    : Node("command_printer")
    {
        subscription_ = this->create_subscription<std_msgs::msg::Int32>(
            "/key_command", 10,
            std::bind(&CommandPrinter::commandCallback, this, std::placeholders::_1));
        
        RCLCPP_INFO(this->get_logger(), "Command Printer started - listening for key commands");
    }

private:
    void commandCallback(const std_msgs::msg::Int32::SharedPtr msg)
    {
        int command = msg->data;
        
        switch(command) {
            case 1:
                std::cout << "1" << std::endl;
                break;
            case 2:
                std::cout << "2" << std::endl;
                break;
            case 3:
                std::cout << "3" << std::endl;
                break;
            case 4:
                std::cout << "4" << std::endl;
                break;
            case 0:
                std::cout << "0" << std::endl;
                break;
            default:
                break;
        }
    }
    
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr subscription_;
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    
    // Create both nodes
    auto key_listener = std::make_shared<key_listener::KeyListener>();
    auto command_printer = std::make_shared<CommandPrinter>();
    
    // Create executor and add both nodes
    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(key_listener);
    executor.add_node(command_printer);
    
    // Spin
    executor.spin();
    
    rclcpp::shutdown();
    return 0;
}