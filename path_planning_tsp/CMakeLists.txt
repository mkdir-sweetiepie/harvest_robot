cmake_minimum_required(VERSION 3.5)
project(tsp)

# 중요: 빌드 디렉토리를 현재 포함 경로에 추가
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 명시적으로 uic 파일 경로 설정 (이 부분이 가장 중요)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# Qt 자동 설정
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 의존성 패키지 찾기
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(vision_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Boost 1.71.0 REQUIRED COMPONENTS thread)

# Qt 라이브러리 찾기
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5DataVisualization REQUIRED)

# 디버그 정보 출력
message(STATUS "Qt5DataVisualization_FOUND: ${Qt5DataVisualization_FOUND}")
message(STATUS "CMAKE_AUTOUIC: ${CMAKE_AUTOUIC}")
message(STATUS "CMAKE_AUTOUIC_SEARCH_PATHS: ${CMAKE_AUTOUIC_SEARCH_PATHS}")

# 소스 파일 지정
set(SOURCES
  src/main.cpp
  src/main_window.cpp
  src/qnode.cpp
)

set(HEADERS
  include/tsp/main_window.hpp
  include/tsp/qnode.hpp
)

set(UI_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/mainwindow.ui
)

# 디버그 정보 출력
message(STATUS "UI files: ${UI_FILES}")

# UI 파일 복사 - 빌드 문제 해결을 위한 임시 방편
file(COPY ${UI_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# UI 파일을 명시적으로 처리
qt5_wrap_ui(UI_HEADERS ${UI_FILES})
message(STATUS "Generated UI headers: ${UI_HEADERS}")

# 리소스 파일 처리
set(RESOURCES
  resources/images.qrc
)
qt5_add_resources(RESOURCE_HEADERS ${RESOURCES})

# 포함 디렉토리 지정
include_directories(
  include/
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Qt5DataVisualization_INCLUDE_DIRS}
)

# 실행 파일 생성
add_executable(${PROJECT_NAME}
  ${SOURCES}
  ${HEADERS}
  ${UI_HEADERS}
  ${RESOURCE_HEADERS}
)

# ROS2 의존성 설정
ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  vision_msgs
  std_msgs
  Boost
)

# Qt 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
  Qt5::Widgets
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  ${Qt5DataVisualization_LIBRARIES}
)

# Qt DataVisualization 컴파일 정의 추가
target_compile_definitions(${PROJECT_NAME} PRIVATE ${Qt5DataVisualization_DEFINITIONS})

# 설치 설정
install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  resources
  ui
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
