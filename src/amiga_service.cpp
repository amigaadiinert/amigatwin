#include <rclcpp/rclcpp.hpp>
#include <motionservice/srv/mode_control.hpp>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <memory>

class KeyboardServiceController : public rclcpp::Node
{
public:
    KeyboardServiceController() : Node("keyboard_service_controller")
    {
        // Create client for the service
        client_ = create_client<motionservice::srv::ModeControl>("/amiga_controller/set_mode");
        
        RCLCPP_INFO(get_logger(), "Keyboard Service Controller Started!");
        RCLCPP_INFO(get_logger(), "Press 'w' for forward mode");
        RCLCPP_INFO(get_logger(), "Press 's' for backward mode");
        RCLCPP_INFO(get_logger(), "Press 'x' for stop mode");
        RCLCPP_INFO(get_logger(), "Press 'q' to quit");
        
        // Timer for keyboard input
        timer_ = create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&KeyboardServiceController::checkInput, this));
    }

private:
    char getch()
    {
        char buf = 0;
        struct termios old {};
        tcgetattr(0, &old);
        
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        
        tcsetattr(0, TCSANOW, &old);
        read(0, &buf, 1);
        
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
        
        return buf;
    }

    void checkInput()
    {
        char c = getch();
        std::string mode;
        
        switch(c)
        {
        case 'w':
        case 'W':
            mode = "forward";
            break;
            
        case 's':
        case 'S':
            mode = "backward";
            break;
            
        case 'x':
        case 'X':
            mode = "stop";
            break;
            
        case 'q':
        case 'Q':
            // Send stop command before quitting
            sendModeRequest("stop");
            RCLCPP_INFO(get_logger(), "Shutting down...");
            rclcpp::shutdown();
            return;
            
        default:
            return;
        }
        
        sendModeRequest(mode);
    }

    void sendModeRequest(const std::string& mode)
    {
        auto request = std::make_shared<motionservice::srv::ModeControl::Request>();
        request->mode = mode;
        
        auto future = client_->async_send_request(request);
        
        // Wait for response
        if (rclcpp::spin_until_future_complete(
                this->shared_from_this(), future) == rclcpp::FutureReturnCode::SUCCESS)
        {
            auto response = future.get();
            RCLCPP_INFO(get_logger(), "%s", response->message.c_str());
        } else {
            RCLCPP_ERROR(get_logger(), "Failed to call service");
        }
    }

    rclcpp::Client<motionservice::srv::ModeControl>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<KeyboardServiceController>());
    rclcpp::shutdown();
    return 0;
}