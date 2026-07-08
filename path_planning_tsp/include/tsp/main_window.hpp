/**
 * @file /include/tsp/main_window.hpp
 *
 * @brief Qt based gui for TSP with ROS2 integration.
 *
 * @date January 2025
 **/

#ifndef TSP_MAIN_WINDOW_HPP
#define TSP_MAIN_WINDOW_HPP

/*****************************************************************************
** Includes
*****************************************************************************/

#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QVector>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtDataVisualization/QScatterDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <unordered_map>

#include "qnode.hpp"

namespace Ui {
class MainWindow;
}

// 3차원 점 클래스
struct Point3D {
  float x, y, z;  // 3D 좌표값
  QString name;   // 점(참외) 이름

  Point3D(float _x = 0, float _y = 0, float _z = 0, const QString& _name = "") : x(_x), y(_y), z(_z), name(_name) {}
};

// 상태 해시 함수 - Held-Karp 알고리즘에서 사용
struct StateHash {
  std::size_t operator()(const std::pair<int, int>& p) const { return std::hash<int>()(p.first) ^ std::hash<int>()(p.second); }
};

/**
 * @class MainWindow
 * @brief 애플리케이션의 메인 윈도우 클래스
 *
 * 참외 수확 경로 최적화 프로그램의 메인 UI와 기능을 구현함
 * Held-Karp 알고리즘을 사용한 TSP(Traveling Salesman Problem) 해결
 * ROS2 통합
 */

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private slots:
  void addPoint();                               // 새 참외(점)을 추가하는 슬롯
  void removePoint();                            // 선택한 참외(점)을 제거하는 슬롯
  void randomizePoints();                        // 랜덤 좌표 생성 함수 추가
  void calculateOptimalPath();                   // 최적 경로 계산 슬롯
  void resetAll();                               // 모든 데이터 초기화 슬롯
  void onTableDataChanged(int row, int column);  // 테이블 데이터 변경 이벤트 처리
  void processCropData();                        // 수신된 참외 데이터 처리
  void requestCropData();                        // 참외 데이터 요청 버튼 클릭 핸들러
  void publishOptimalPath();                     // 최적 경로 발행 슬롯

 private:
  void closeEvent(QCloseEvent* event) override;  // 윈도우 닫기 이벤트 처리
  void setupUI();                                // UI 설정 함수
  void setupConnections();                       // 시그널/슬롯 연결 함수
  void updateVisualization();                    // 시각화 업데이트 함수
  void updatePathVisualization();                // 경로 시각화 업데이트 함수
  void addDefaultPoints();                       // 기본 점 추가 함수

  Ui::MainWindow* ui;  // UI 객체 포인터
  QNode* qnode;        // ROS2 노드 객체

  QTableWidget* coordTable;        // 좌표 테이블 위젯
  QPushButton* addPointButton;     // 참외 추가 버튼
  QPushButton* removePointButton;  // 참외 제거 버튼
  QPushButton* randomizeButton;    // 랜덤 버튼 추가
  QPushButton* calculateButton;    // 최적 경로 계산 버튼
  QPushButton* resetButton;        // 초기화 버튼
  QPushButton* cropDataButton;     // 감지된 참외 데이터 요청 버튼
  QLabel* resultLabel;             // 결과 표시 레이블
  QLabel* statusLabel;             // 상태 표시 레이블
  QPushButton* publishButton;      // 최적 경로 퍼블리시 버튼

  Point3D manipulatorPosition;  // 매니퓰레이터 시작 위치

  QtDataVisualization::Q3DScatter* scatter3D;                // 3D 산점도 시각화 객체
  QtDataVisualization::QScatter3DSeries* pointSeries;        // 점 시리즈
  QtDataVisualization::QScatter3DSeries* pathSeries;         // 경로 시리즈
  QtDataVisualization::QScatter3DSeries* orderSeries;        // 순서 시리즈
  QtDataVisualization::QScatter3DSeries* manipulatorSeries;  // 매니퓰레이터 시각화를 위한 시리즈

  QVector<Point3D> points;   // 3D 점 배열
  QVector<int> optimalPath;  // 최적 경로 인덱스 배열
  double minCost;            // 최적 경로 비용

  bool isInitializing = false;  // 초기화 플래그 추가
};

#endif  // TSP_MAIN_WINDOW_HPPu