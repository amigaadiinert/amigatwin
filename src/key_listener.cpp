#include "key_listener/key_listener.hpp"
#include <iostream>

namespace key_listener
{

KeyListener::KeyListener() 
: KeyListener(rclcpp::NodeOptions()) 
{}

KeyListener::KeyListener(const rclcpp::NodeOptions & options)
: Node("key_listener", options),
  terminal_configured_(false)
{
    // Publisher for key commands
    key_pub_ = this->create_publisher<std_msgs::msg::Int32>("/key_command", 10);
    
    // Setup terminal for non-blocking input
    setupTerminal();
    
    // Create timer to check for key presses (10 Hz)
    timer_ = create_wall_timer(
        std::chrono::milliseconds(100),
        std::bind(&KeyListener::timerCallback, this));
    
    RCLCPP_INFO(this->get_logger(), "Key Listener started - Press any key (Ctrl+C to exit)");
}

KeyListener::~KeyListener()
{
    restoreTerminal();
}

void KeyListener::setupTerminal()
{
    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &original_termios_);
    
    // Create new settings
    struct termios new_termios = original_termios_;
    
    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    
    // Set minimum read to 0 (non-blocking)
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;
    
    // Apply settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    // Set stdin to non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    terminal_configured_ = true;
}

void KeyListener::restoreTerminal()
{
    if (terminal_configured_) {
        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios_);
        
        // Restore blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
        
        terminal_configured_ = false;
    }
}

char KeyListener::getKey()
{
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) > 0) {
        return c;
    }
    return 0;
}

void KeyListener::timerCallback()
{
    char key = getKey();
    
    if (key != 0) {
        auto msg = std_msgs::msg::Int32();
        
        // Handle command keys
        if (key == 'w' || key == 'W') {
            msg.data = 1;  // Forward
            key_pub_->publish(msg);
        } else if (key == 's' || key == 'S') {
            msg.data = 2;  // Backward
            key_pub_->publish(msg);
        } else if (key == 'a' || key == 'A') {
            msg.data = 3;  // Left
            key_pub_->publish(msg);
        } else if (key == 'd' || key == 'D') {
            msg.data = 4;  // Right
            key_pub_->publish(msg);
        } else if (key == 'x' || key == 'X') {
            msg.data = 0;  // Stop
            key_pub_->publish(msg);
        } else if (key == 3) {  // Ctrl+C
            RCLCPP_INFO(this->get_logger(), "Ctrl+C detected - exiting");
            rclcpp::shutdown();
        }
        // Other keys are ignored - no message published
    }
}

}  // namespace key_listener