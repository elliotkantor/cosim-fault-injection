#!/bin/bash

mkdir -p results/
TIMESTAMP=$(date +%s)

BROKER_LOG="./results/broker_$TIMESTAMP.log"
TRANSMISSION_LOG="./results/TransmissionSim_$TIMESTAMP.log"
CC_LOG="./results/CCSim_$TIMESTAMP.log"
DISTRIBUTION_LOG="./results/DistributionSim_$TIMESTAMP.log"
RELAY_LOG="./results/RelaySim_$TIMESTAMP.log"
NAV2_LOG="./results/NAV2_$TIMESTAMP.log"
GAZEBO_LOG="./results/Gazebo_$TIMESTAMP.log"
ROS2_LOG="./results/ROS2_$TIMESTAMP.log"

touch $BROKER_LOG
touch $TRANSMISSION_LOG
touch $DISTRIBUTION_LOG
touch $CC_LOG
touch $RELAY_LOG
touch $NAV2_LOG
touch $GAZEBO_LOG
touch $ROS2_LOG

uv sync

# Source ROS2 setup
source /opt/ros/humble/setup.bash
# If using NAV2 workspace
if [ -f ~/cosim-fault-injection/NAV2/src/install/setup.bash ]; then
  source ~/cosim-fault-injection/NAV2/src/install/setup.bash
fi

HELICS_BROKER=`which helics_broker`
($HELICS_BROKER -t="zmq" --federates=5 --name=mainbroker > $BROKER_LOG)&

# Start Gazebo and NAV2 stack
echo "Starting Gazebo and NAV2 stack..."
ros2 launch vipnav launch_sim.launch.py > $GAZEBO_LOG 2>&1 &
NAV2_PID=$!
sleep 5

# Give NAV2 time to start up
sleep 5

# Start grid and power simulators
cd Transmission
uv run Transmission_simulator.py > ../$TRANSMISSION_LOG 2>&1 &
cd ..

cd CC
uv run CC_simulator.py > ../$CC_LOG 2>&1 &
cd ..

cd Distribution
gridlabd IEEE_123_feeder_0.glm > ../$DISTRIBUTION_LOG 2>&1 &
cd ..

cd Relay 
uv run Relay_simulator.py > ../$RELAY_LOG 2>&1 &
cd ..

# Start ROS2 minimal publisher (HELICS federate for NAV2)
echo "Starting ROS2 HELICS federate..."
ros2 run examples_rclcpp_minimal_publisher publisher_member_function > $ROS2_LOG 2>&1 &

echo "All simulators and ROS2 services started!"
echo "Broker PID: $HELICS_BROKER"
echo "NAV2/Gazebo PID: $NAV2_PID"
echo "Logs available in results/ directory"
echo ""
echo "To stop all services, run: pkill -f 'helics_broker|gazebo|ros2|gridlabd|Python'; sleep 2"
