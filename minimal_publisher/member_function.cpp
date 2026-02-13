// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <functional>
#include <iostream>
#include <string.h>
#include <memory>
#include <string>
#include <map>
#include <complex>

#include "rclcpp/rclcpp.hpp"
#include "helics/cpp98/helics.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using namespace helicscpp;

/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
public:
  MinimalPublisher()
  : Node("minimal_publisher"), count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
    timer_ = this->create_wall_timer(
      100ms, std::bind(&MinimalPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    auto message = std_msgs::msg::String();
    auto message1 = std_msgs::msg::String();

    // Create value federate from configuration file
    ValueFederate fed ("ROS2_config_cc.json");
    fed.registerInterfaces("ROS2_config_cc.json");
    const char* federateName = fed.getName();
    message.data = "HELICS Version: " + std::string(helicsGetVersion());
    RCLCPP_INFO(this->get_logger(), message.data.c_str());
    message.data = std::string(federateName) + ": Federate " + std::string(federateName) + " has been registered";
    RCLCPP_INFO(this->get_logger(), message.data.c_str());
    int pubKeysCount = fed.getPublicationCount();
    int subKeysCount = fed.getInputCount();
    // Reference to Publications and Subscription form index
    std::map<std::string, Publication> pubId;
    std::map<std::string, Input> subId;
    for (int i = 0; i < pubKeysCount; ++i) {
        Publication pub = fed.getPublication(i);
        pubId["m" + std::to_string(i)] = pub;
        const char* pubName = pub.getName();
	message.data = std::string(federateName) + ": Registered Publication ---> " + std::string(pubName);
        RCLCPP_INFO(this->get_logger(), message.data.c_str());
    }
    for (int i = 0; i < subKeysCount; ++i) {
        Input sub = fed.getInput(i);
        sub.setDefault(0.0);
        subId["m" + std::to_string(i)] = sub;
        const char* subKey = sub.getTarget();
	message.data = std::string(federateName) + ": Registered Subscription ---> " + std::string(subKey);
        RCLCPP_INFO(this->get_logger(), message.data.c_str());
    }

    // Entering Execution Mode
    fed.enterInitializingMode();
    auto status = fed.enterExecutingMode();
    
    int grantedTime = -1;
    int hours = 24;
    int totalInterval = 60 * 60 * hours;
    int sensingInterval = 5 * 60;

    // Starting Co-simulation
    for (int t = 0; t < totalInterval; t += sensingInterval) {
        while (grantedTime < t) {
            grantedTime = fed.requestTime(t);
        }
	for (int i = 0; i < subKeysCount; ++i) {
            Input sub = subId["m" + std::to_string(i)];
	    std::string name = sub.getName();
            auto report = sub.getString();
	    message1.data = std::string(federateName) + ": " + std::string(report);
            RCLCPP_INFO(this->get_logger(), message1.data.c_str());
	}
	message.data = std::string(federateName) + ": Federate Granted Time = " + std::to_string(grantedTime);
	RCLCPP_INFO(this->get_logger(), "%s", message.data.c_str());
        std::string helicsVersion = helicsGetVersion();
        message.data = "Hello, world! " + std::to_string(count_++);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s' and '%s'", message1.data.c_str(), helicsVersion.c_str());
        publisher_->publish(message1);
    }

    // Terminating Federate
    constexpr int terminationTime = 60 * 60 * 24;
    while (grantedTime < terminationTime) {
        grantedTime = fed.requestTime(terminationTime);
    }
    message.data = std::string(federateName) + ": Destroying federate";
    RCLCPP_INFO(this->get_logger(), message.data.c_str());
    fed.finalize();
    helicsCloseLibrary();
    message.data = std::string(federateName) + ": Done!";
    RCLCPP_INFO(this->get_logger(), message.data.c_str());
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}
