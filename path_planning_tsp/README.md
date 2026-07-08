# TSP (Traveling Salesman Problem) Solver for ROS2

![TSP Solver Visualization](https://github.com/user-attachments/assets/a1002c61-1530-4454-a97f-fd761a2992cd)

## Overview

This package implements a Traveling Salesman Problem (TSP) solver with visualization capabilities for ROS2. It provides an efficient algorithm to find the shortest possible route that visits a set of points exactly once and returns to the origin point.

## Features

- Fast TSP optimization algorithms
- Real-time 3D visualization of routes and solutions
- Integration with ROS2 ecosystem
- Support for dynamic point updates
- Multiple solver options (nearest neighbor, genetic algorithm, etc.)

## Dependencies

- ROS2 (tested on Humble and Foxy)
- Qt5 Data Visualization library
- C++14 or higher

## Installation

### 1. Install required dependencies

```bash
sudo apt update
sudo apt install libqt5datavisualization5-dev
```

### 2. Clone the repository into your ROS2 workspace

```bash
cd ~/ros2_ws/src/
git clone https://github.com/yourusername/tsp.git
```

### 3. Build the package

```bash
cd ~/ros2_ws/
colcon build --packages-select tsp
source install/setup.bash
```

## Usage

### Basic Execution

To run the TSP solver with visualization:

```bash
ros2 run tsp tsp
```

## Topics

### Subscribed Topics
- `/detected_crops` (vision_msgs::msg::DetectedCropArray): Input points for TSP calculation

### Published Topics
- `/harvest_ordering` (vision_msgs::msg::HarvestOrdering): The calculated optimal ordering for harvest

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.


## Contact

For any questions or suggestions, please open an issue on t
