# Crop Harvesting Robot - Integrated Control System

This project is an integrated control system for the crop harvesting robot developed by BARAM Robotics Club at Kwangwoon University. It serves as a control tower based on Qt GUI, integrating and controlling all subsystems including vision system, motor control, and path planning.

## Key Features

### Main Control
- **Automatic Harvest Sequence Execution**: Automatically manages the entire harvesting process
- **Real-time Progress Monitoring**: Displays current stage and completion rate
- **Emergency Stop Function**: Immediate stop function for safety
- **System Initialization**: Complete system state reset

### Vision System
- **YOLOv12-X Based Melon Detection**: Distinguishes ripe melons (8) from unripe melons (2)
- **FoundationPose 6D Estimation**: Precise melon position and orientation calculation
- **Camera Calibration**: Intrinsic/extrinsic parameters and hand-eye calibration

### Motor Control
- **6DOF Manipulator Control**: Precise control of 6-axis robot arm
- **Manual Position Control**: Direct position commands through GUI
- **DC Motor + Dynamixel Integration**: Control of 4 DC motors + 2 Dynamixel servos
- **Real-time Status Monitoring**: Motor status and position feedback

### Path Planning
- **TSP Optimization**: Optimal harvest order determination using Held-Karp algorithm
- **RRT* Path Generation**: Safe path planning considering obstacle avoidance
- **Spline Interpolation**: Smooth trajectory generation

## System Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                    Control Tower GUI (Qt)                   │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────┐│
│  │ Main Control│ │Vision System│ │Motor Control│ │Path Plan││
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────┘│
└─────────────────────────┬───────────────────────────────────┘
                         │ ROS 2 Communication
        ┌────────────────┼────────────────┐
        │                │                │
   ┌─────────┐    ┌─────────────┐    ┌─────────┐
   │Vision   │    │ Path Plan   │    │Control  │
   │Node     │    │ Node        │    │Node     │
   └─────────┘    └─────────────┘    └─────────┘
        │                │                │
   ┌─────────┐    ┌─────────────┐    ┌─────────┐
   │YOLOv12-X│    │  TSP + RRT* │    │Hardware │
   │Foundation│   │             │    │STM32+U2D2│
   └─────────┘    └─────────────┘    └─────────┘
```

## Installation and Build

### 1. System Requirements
- Ubuntu 22.04 LTS
- ROS 2 Humble Hawksbill
- Qt5 (5.15 or higher)
- OpenCV 4.x
- Python 3.10+

### 2. Install Dependencies
```bash
# Install ROS 2 (skip if already installed)
sudo apt update
sudo apt install ros-humble-desktop

# Install Qt5 development tools
sudo apt install qtbase5-dev qttools5-dev-tools

# Additional ROS 2 packages
sudo apt install ros-humble-joint-state-publisher
sudo apt install ros-humble-robot-state-publisher
sudo apt install ros-humble-gazebo-ros-pkgs

# Python dependencies
pip3 install torch torchvision ultralytics
pip3 install opencv-python numpy
```

### 3. Workspace Setup
```bash
# Create workspace
mkdir -p ~/harvest_ws/src
cd ~/harvest_ws/src

# Clone source code
git clone <repository-url> harvest_master

# Build
cd ~/harvest_ws
colcon build --packages-select harvest_master

# Environment setup
source install/setup.bash
```

## Execution

### 1. Real Robot System
```bash
# Run full system
ros2 launch harvest_master harvest_master_launch.py

# Run individual node (for testing)
ros2 run harvest_master harvest_master
```

### 2. Simulation Environment
```bash
# Run with Gazebo simulation
ros2 launch harvest_master simulation_launch.py
```

## Usage

### Main Control Tab
1. **Check System Status**: Verify each subsystem status in the top status indicator
2. **Start Harvest**: Click "Start Harvest" button to execute automatic harvest sequence
3. **Monitor Progress**: Check real-time progress through the progress bar
4. **Emergency**: Press "Emergency Stop" button to immediately stop the system

### Vision System Tab
1. **Manual Detection**: Test melon detection with "Start Manual Detection" button
2. **Check Results**: View detected melon list and coordinate information
3. **Calibration**: Execute camera calibration

### Motor Control Tab
1. **Manual Control**: Enter X, Y, Z coordinates and rotation angles, then click "Move to Position"
2. **Test Functions**: Test cutting tool and move to home position
3. **Status Monitoring**: Check real-time motor status

### Path Planning Tab
1. **Harvest Order**: View optimal harvest order calculated by TSP algorithm
2. **Current Target**: Display information about the target melon in progress
3. **Recalculate Path**: Execute path recalculation when needed


## Development Team

### BARAM Robotics Club (Kwangwoon University)
- **Team Leader**: Donggyun Lim (30th) - Manipulator Control
- **Member**: Gahyeon Oh (32nd) - Manipulator Simulation
- **Member**: Jihyeon Hong (32nd) - Computer Vision & SLAM
- **Member**: Wookyung Jung (33rd) - Computer Vision (Team Leader)

### Contact
- **Email**: baram@kw.ac.kr
- **Website**: baram.kw.ac.kr
- **GitHub**: [Project Repository URL]


## Technical Details

### ROS 2 Topic Structure
```bash
# Published Topics
/system_state          (std_msgs/String)
/camera_trigger        (std_msgs/Bool)
/joint_velocity        (std_msgs/Float64MultiArray)
/path_planning_request (geometry_msgs/Pose)
/cutting_trigger       (std_msgs/Bool)
/system_command        (std_msgs/String)

# Subscribed Topics
/detected_crops        (geometry_msgs/PoseArray)
/harvest_order         (std_msgs/Float64MultiArray)
/planned_path          (std_msgs/Float64MultiArray)
/crop_6d_pose          (geometry_msgs/Pose)
/robot_status          (std_msgs/String)
/vision_status         (std_msgs/String)
/motor_status          (std_msgs/String)
/planning_status       (std_msgs/String)
```

### State Machine
```cpp
enum class RobotState {
    INIT,              // Initialization
    DETECTION,         // Melon detection
    HARVEST_PLANNING,  // Harvest planning
    APPROACHING,       // Approaching
    FOUNDATION_POSE,   // Stem estimation
    CUTTING,          // Cutting
    NEXT_TARGET,      // Next target
    COMPLETED,        // Completed
    ERROR             // Error
};
```

