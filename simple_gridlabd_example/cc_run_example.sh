#!/bin/bash

mkdir -p results/
TIMESTAMP=$(date +%s)

BROKER_LOG="./results/broker_$TIMESTAMP.log"
TRANSMISSION_LOG="./results/TransmissionSim_$TIMESTAMP.log"
CC_LOG="./results/CCSim_$TIMESTAMP.log"
DISTRIBUTION_LOG="./results/DistributionSim_$TIMESTAMP.log"
RELAY_LOG="./results/RelaySim_$TIMESTAMP.log"

touch $BROKER_LOG
touch $TRANSMISSION_LOG
touch $DISTRIBUTION_LOG
touch $CC_LOG
touch $RELAY_LOG

uv sync

HELICS_BROKER=`which helics_broker`
($HELICS_BROKER -t="zmq" --federates=5 --name=mainbroker > $BROKER_LOG)&

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
