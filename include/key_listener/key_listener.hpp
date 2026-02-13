#ifndef KEY_LISTENER__KEY_LISTENER_HPP_
#define KEY_LISTENER__KEY_LISTENER_HPP_

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

namespace key_listener
{

class KeyListener : public rclcpp::Node
{
public:
    KeyListener();
    explicit KeyListener(const rclcpp::NodeOptions & options);
    virtual ~KeyListener();

private:
    void timerCallback();
    char getKey();
    void setupTerminal();
    void restoreTerminal();
    
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr key_pub_;
    struct termios original_termios_;
    bool terminal_configured_;
};

}  // namespace key_listener

#endif  // KEY_LISTENER__KEY_LISTENER_HPP_
