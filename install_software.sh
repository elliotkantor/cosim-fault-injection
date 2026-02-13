#!/bin/bash
set -e

SOFTWARE=/software

# ---- ROS ----
if [ ! -d "/home/ubuntu/ros2_ws/src/examples" ]; then
  cd ~/ros2_ws
  git clone https://github.com/ros2/examples src/examples -b humble
  colcon build --symlink-install || true
  echo 'source install/setup.bash' >> ~/.bashrc  # every environment should have ROS for convenience
fi


# ---- HELICS ----
if [ ! -d "$SOFTWARE/HELICS" ]; then
  git clone --branch v3.5.1 --single-branch https://github.com/GMLC-TDC/HELICS $SOFTWARE/HELICS
fi

cd $SOFTWARE/HELICS
git checkout v3.5.1

mkdir -p build
cd build

cmake \
  -DHELICS_BUILD_CXX_SHARED_LIB=ON \
  -DCMAKE_INSTALL_PREFIX=$SOFTWARE/HELICS/install \
  -DCMAKE_CXX_STANDARD=20 \
  ..

make -j$(nproc)
make install

# add to bashrc once (may leave duplicates if run multiple times)
echo "export PATH=${SOFTWARE}/HELICS/install/bin:\$PATH" >> ~/.bashrc
echo "export CPLUS_INCLUDE_PATH=\$CPLUS_INCLUDE_PATH:${SOFTWARE}/HELICS/include" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=${SOFTWARE}/HELICS/install/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc

# ---- GridLAB-D ----
cd $SOFTWARE

if [ ! -d "gridlab-d" ]; then
  git clone --branch release/5.1 --single-branch https://github.com/gridlab-d/gridlab-d.git
fi

cd gridlab-d
git submodule update --init

mkdir -p build
cd build

cmake \
  -DCMAKE_INSTALL_PREFIX=$SOFTWARE/GridLAB-D \
  -DGLD_USE_HELICS=ON \
  -DGLD_HELICS_DIR=$SOFTWARE/HELICS/install \
  ..

make -j$(nproc)
make install

# add to bashrc once (may leave duplicates if run multiple times)
echo "export PATH=${SOFTWARE}/GridLAB-D/bin:\$PATH" >> ~/.bashrc
echo "export GLPATH=${SOFTWARE}/GridLAB-D/share" >> ~/.bashrc

# finally source it
source ~/.bashrc

# OPTIONAL: download relevant example
cd ~
git clone https://github.com/cps-vip/cosim-fault-injection.git

echo "INSTALL COMPLETE"

