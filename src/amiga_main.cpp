#include <rclcpp/rclcpp.hpp>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <thread>
#include <sys/wait.h>

class ControllerManager : public rclcpp::Node
{
public:
    ControllerManager() : Node("controller_manager"), robot_process_pid_(-1)
    {
        RCLCPP_INFO(get_logger(), "Controller Manager Started!");
        RCLCPP_INFO(get_logger(), "Press 'w' to start robot controller");
        RCLCPP_INFO(get_logger(), "Press 's' to start robot controller");
        RCLCPP_INFO(get_logger(), "Press 'x' to stop robot controller");
        RCLCPP_INFO(get_logger(), "Press 'q' to quit");
        
        // Create a timer for checking keyboard input
        input_timer_ = create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&ControllerManager::checkInput, this));
    }

private:
    char getch()
    {
        char buf = 0;
        struct termios old {};
        tcgetattr(0, &old); 
        
        if (tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
        
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        
        if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
        
        if (read(0, &buf, 1) < 0)
            perror("read()");
        
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
            perror("tcsetattr ~ICANON");
        
        return buf;
    }

    void checkInput()
    {
        char c = getch();
        
        switch(c)
        {
        case 'w':
        case 'W':
            startForwardController();
            break;
            
        case 'x':
        case 'X':
            stopController();
            break;

        case 's':
        case 'S':
            startBackwardController();
            break;
            
        case 'q':
        case 'Q':
            // Cleanup before exiting
            stopController();
            RCLCPP_INFO(get_logger(), "Shutting down...");
            rclcpp::shutdown();
            break;
            
        default:
            // Ignore other keys
            break;
        }
    }

    void startForwardController()
    {
        if (robot_process_pid_ > 0)
        {
            RCLCPP_WARN(get_logger(), "Forward controller is already running!");
            return;
        }
        
        RCLCPP_INFO(get_logger(), "Starting forward controller...");
        
        // Fork a new process to run the robot controller
        pid_t pid = fork();
        
        if (pid == 0)
        {
            // Child process - execute the robot controller
            execlp("ros2", "ros2", "run", "amiga", "forward_control", NULL);
            
            // If execlp fails
            RCLCPP_ERROR(get_logger(), "Failed to start forward controller!");
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            robot_process_pid_ = pid;
            RCLCPP_INFO(get_logger(), "Forward controller started with PID: %d", pid);
        }
        else
        {
            // Fork failed
            RCLCPP_ERROR(get_logger(), "Failed to fork process!");
        }
    }

    void stopController()
    {
        if (robot_process_pid_ <= 0)
        {
            RCLCPP_WARN(get_logger(), "No robot controller is running!");
            return;
        }
        
        RCLCPP_INFO(get_logger(), "Stopping robot controller");
        
        // Fork a new process to run the robot controller
        pid_t pid = fork();
        
        if (pid == 0)
        {
            // Child process - execute the robot controller
            execlp("ros2", "ros2", "run", "amiga", "stopper", NULL);
            
            // If execlp fails
            RCLCPP_ERROR(get_logger(), "Failed to stop controller!");
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            robot_process_pid_ = pid;
        }
        else
        {
            // Fork failed
            RCLCPP_ERROR(get_logger(), "Failed to fork process!");
        }
    }

    void startBackwardController()
    {
        if (robot_process_pid_ > 0)
        {
            RCLCPP_WARN(get_logger(), "Backward controller is already running!");
            return;
        }
        
        RCLCPP_INFO(get_logger(), "Starting backward controller...");
        
        // Fork a new process to run the robot controller
        pid_t pid = fork();
        
        if (pid == 0)
        {
            // Child process - execute the robot controller
            execlp("ros2", "ros2", "run", "amiga", "backward_control", NULL);
            
            // If execlp fails
            RCLCPP_ERROR(get_logger(), "Failed to start backward controller!");
            exit(1);
        }
        else if (pid > 0)
        {
            // Parent process
            robot_process_pid_ = pid;
            RCLCPP_INFO(get_logger(), "Backward controller started with PID: %d", pid);
        }
        else
        {
            // Fork failed
            RCLCPP_ERROR(get_logger(), "Failed to fork process!");
        }
    }

    // ROS components
    rclcpp::TimerBase::SharedPtr input_timer_;
    
    // Process management
    pid_t robot_process_pid_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ControllerManager>());
    return 0;
}