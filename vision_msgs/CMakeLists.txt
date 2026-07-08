cmake_minimum_required(VERSION 3.8)
project(vision_msgs)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# 비전 시스템 관련 메시지 목록
set(VISION_MSGS
  "msg/CropPose.msg"
  "msg/DetectedCrop.msg"
  "msg/DetectedCropArray.msg"
  "msg/HarvestOrdering.msg"
)

# Build vision_msgs_node
# 커스텀 메시지 생성
rosidl_generate_interfaces(${PROJECT_NAME}
  ${VISION_MSGS}
  DEPENDENCIES std_msgs rclcpp geometry_msgs
)

ament_package()
