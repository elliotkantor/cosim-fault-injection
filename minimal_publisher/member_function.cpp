#include <memory>
#include <string>
#include <map>
#include <iostream>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "/home/ubuntu/software/HELICS/src/helics/cpp98/helics.hpp"

using namespace std::chrono_literals;
using namespace helicscpp;

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher()
    : Node("minimal_publisher"), count_(0)
    {
        // ROS2 publisher
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);

        // HELICS ValueFederate from your config
        fed_ = std::make_unique<ValueFederate>("/home/ubuntu/ros2_ws/ROS2_config_cc.json");
        fed_->registerInterfaces("/home/ubuntu/ros2_ws/ROS2_config_cc.json");

        // Register publications/subscriptions
        int pubCount = fed_->getPublicationCount();
        int subCount = fed_->getInputCount();
        for (int i = 0; i < pubCount; ++i) {
            pubId_["m" + std::to_string(i)] = fed_->getPublication(i);
        }
        for (int i = 0; i < subCount; ++i) {
            Input sub = fed_->getInput(i);
            sub.setDefault(0.0);
            subId_["m" + std::to_string(i)] = sub;
        }

        // Enter HELICS execution mode
        fed_->enterInitializingMode();
        fed_->enterExecutingMode();

        // ROS2 timer to periodically request HELICS time and publish
        timer_ = this->create_wall_timer(100ms, std::bind(&MinimalPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
        std_msgs::msg::String msg;

        // Minimal HELICS time request logic
        static int grantedTime = -1;
        int t = grantedTime + 60; // advance by 60s per callback
        grantedTime = fed_->requestTime(t);

        // Iterate over subscriptions to check for relay trip/fault
        for (auto& [key, sub] : subId_) {
            std::string report = sub.getString(); // get trip/fault info
            msg.data = report.empty() ? "No trip" : report;
            RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
            publisher_->publish(msg);
        }
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::unique_ptr<ValueFederate> fed_;
    std::map<std::string, Publication> pubId_;
    std::map<std::string, Input> subId_;
    size_t count_;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}