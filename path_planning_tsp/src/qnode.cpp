/**
 * @file /src/qnode.cpp
 *
 * @brief Ros communication central!
 *
 * @date January 2025
 **/

/*****************************************************************************
** Includes
*****************************************************************************/

#include "../include/tsp/qnode.hpp"

#include "../include/tsp/main_window.hpp"  // Point3D 구조체 사용을 위해 추가

QNode::QNode() {
  int argc = 0;
  char** argv = NULL;
  rclcpp::init(argc, argv);
  node = rclcpp::Node::make_shared("tsp");

  // 참외 감지 데이터 구독 (calibration 결과)
  crop_subscription = node->create_subscription<vision_msgs::msg::DetectedCropArray>("/detected_crops/result", 10,
                                                                                     std::bind(&QNode::cropCallback, this, std::placeholders::_1));

  // 수확 순서 발행자 초기화
  harvest_publisher = node->create_publisher<vision_msgs::msg::HarvestOrdering>("/harvest_ordering/result2", 10);

  this->start();
}

QNode::~QNode() {
  if (rclcpp::ok()) {
    rclcpp::shutdown();
  }
}

void QNode::run() {
  rclcpp::WallRate loop_rate(20);
  while (rclcpp::ok()) {
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }
  rclcpp::shutdown();
  Q_EMIT rosShutDown();
}

void QNode::cropCallback(const vision_msgs::msg::DetectedCropArray::SharedPtr msg) {
  // 수신된 참외 데이터를 그리퍼 좌표계로 변환하여 저장
  crop_data = *msg;

  // 각 참외의 좌표를 그리퍼 좌표계에 맞게 변환
  // calibration node에서 이미 변환된 좌표가 오므로 추가 변환은 불필요
  // 단, 시각화를 위해 좌표계 이해는 필요:
  // 그리퍼 좌표계: X(전진), Y(좌우), Z(상하)
  
  RCLCPP_INFO(node->get_logger(), "Received %d crops in gripper coordinate system", msg->total_objects);
  
  for (const auto& crop : msg->objects) {
    RCLCPP_INFO(node->get_logger(), 
                "Crop ID=%d: X=%.3f m (forward), Y=%.3f m (left/right), Z=%.3f m (up/down)", 
                crop.id, crop.x, crop.y, crop.z);
  }

  // 메인 윈도우에 시그널 전송
  Q_EMIT newCropDataReceived();
}

// 매니퓰레이터 위치를 고려한 수확 순서 발행 메소드 구현
void QNode::publishHarvestOrder(const QVector<int>& path, const QVector<Point3D>& points, const Point3D& manipulatorPos) {
  if (!rclcpp::ok()) {
    return;
  }

  // 메시지 생성
  auto message = std::make_shared<vision_msgs::msg::HarvestOrdering>();

  // 헤더 설정 - 그리퍼 좌표계 기준
  message->header.stamp = node->now();
  message->header.frame_id = "gripper_base";  // 그리퍼 좌표계로 변경

  // 그리퍼 좌표계에서의 참외 객체 정보 복사
  for (const auto& point : points) {
    vision_msgs::msg::DetectedCrop crop;

    // 이름에서 ID 추출 ("참외1" -> 1)
    QString name = point.name;
    bool ok;
    int id = name.mid(2).toInt(&ok);

    if (!ok) {
      // 변환 실패 시 인덱스를 찾아서 ID 설정
      for (int i = 0; i < points.size(); ++i) {
        if (points[i].name == name) {
          id = i + 1;  // 1부터 시작하는 ID 사용
          break;
        }
      }
    }

    crop.id = id;
    // 좌표는 이미 그리퍼 좌표계 기준 (m 단위)
    crop.x = point.x;  // X: 전진/후진
    crop.y = point.y;  // Y: 좌/우
    crop.z = point.z;  // Z: 상/하

    message->objects.push_back(crop);
  }

  // 총 객체 수 설정
  message->total_objects = points.size();

  // 수확 순서 추가 (ID 배열)
  for (int i = 0; i < path.size(); ++i) {
    int pointIndex = path[i];

    // 인덱스 유효성 검사
    if (pointIndex < 0 || pointIndex >= points.size()) {
      continue;
    }

    // 참외 이름에서 ID 추출 ("참외1" -> 1)
    QString name = points[pointIndex].name;
    bool ok;
    int id = name.mid(2).toInt(&ok);

    if (!ok) {
      // 변환 실패 시 인덱스 사용
      id = pointIndex + 1;  // 1부터 시작하는 ID 사용
    }

    message->crop_ids.push_back(id);
  }

  // 매니퓰레이터 시작 위치 정보 로그 출력 (그리퍼 좌표계 기준)
  RCLCPP_INFO(node->get_logger(), 
              "Publishing harvest order with %d crops. Manipulator start position in gripper coords: X=%.3f m (forward), Y=%.3f m (left/right), Z=%.3f m (up/down)", 
              path.size(), manipulatorPos.x, manipulatorPos.y, manipulatorPos.z);

  // 수확 순서 로그 출력
  std::string order_str = "Harvest order: ";
  for (size_t i = 0; i < message->crop_ids.size(); ++i) {
    order_str += std::to_string(message->crop_ids[i]);
    if (i < message->crop_ids.size() - 1) order_str += " -> ";
  }
  RCLCPP_INFO(node->get_logger(), "%s", order_str.c_str());

  // 메시지 발행
  harvest_publisher->publish(*message);
}