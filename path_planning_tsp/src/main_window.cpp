/**
 * @file /src/main_window.cpp
 *
 * @brief Implementation for the qt gui with ROS2 integration.
 *
 * @date January 2025
 **/
/*****************************************************************************
** Includes
*****************************************************************************/

#include "../include/tsp/main_window.hpp"

#include <QGroupBox>                              // 그룹박스를 위해 추가
#include <QHBoxLayout>                            // 수평 레이아웃을 위해 추가
#include <QHeaderView>                            // 헤더 뷰를 위해 추가
#include <QMessageBox>                            // 메시지 박스를 위해 추가
#include <QVBoxLayout>                            // 수직 레이아웃을 위해 추가
#include <QtDataVisualization/Q3DTheme>           // 3D 테마를 위해 추가
#include <QtDataVisualization/QScatterDataProxy>  // 산포 데이터 프록시를 위해 추가
#include <QtDataVisualization/QValue3DAxis>       // 3D 축을 위해 추가
#include <algorithm>                              // 알고리즘 함수를 위해 추가
#include <bitset>                                 // 비트셋을 위해 추가
#include <cmath>                                  // 수학 함수를 위해 추가
#include <limits>                                 // 수치형 타입의 최대/최소값을 위해 추가
#include <random>                                 // 랜덤 기능을 위해 추가

#include "ui_mainwindow.h"

using namespace QtDataVisualization;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), orderSeries(nullptr) {
  ui->setupUi(this);
  setWindowTitle("참외 수확 경로 최적화 (Held-Karp) - 그리퍼 좌표계");

  // 매니퓰레이터 위치 초기화 (그리퍼 좌표계 원점)
  manipulatorPosition = Point3D(0, 0, 0, "그리퍼");

  // ROS2 노드 초기화
  qnode = new QNode();
  QObject::connect(qnode, SIGNAL(rosShutDown()), this, SLOT(close()));
  QObject::connect(qnode, SIGNAL(newCropDataReceived()), this, SLOT(processCropData()));

  // UI 요소 설정
  setupUI();

  // 시그널/슬롯 연결
  setupConnections();

  // 초기 데이터 추가
  addDefaultPoints();

  // 초기 시각화 업데이트
  updateVisualization();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent* event) { QMainWindow::closeEvent(event); }

void MainWindow::setupUI() {
  // 메인 레이아웃 설정
  QWidget* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

  // 왼쪽 패널 (컨트롤)
  QVBoxLayout* leftLayout = new QVBoxLayout();
  leftLayout->setContentsMargins(10, 10, 10, 10);
  leftLayout->setSpacing(10);

  // 좌표 테이블 그룹
  QGroupBox* coordGroupBox = new QGroupBox("참외 3D 좌표 (그리퍼 좌표계)");
  QVBoxLayout* coordLayout = new QVBoxLayout(coordGroupBox);

  coordTable = new QTableWidget(0, 4);
  coordTable->setHorizontalHeaderLabels(QStringList() << "이름" << "X (전진)" << "Y (좌우)" << "Z (상하)");
  coordTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  coordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  coordLayout->addWidget(coordTable);

  // 버튼 영역
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  addPointButton = new QPushButton("참외 추가");
  removePointButton = new QPushButton("참외 제거");
  randomizeButton = new QPushButton("랜덤 좌표");
  buttonLayout->addWidget(addPointButton);
  buttonLayout->addWidget(removePointButton);
  buttonLayout->addWidget(randomizeButton);
  coordLayout->addLayout(buttonLayout);

  leftLayout->addWidget(coordGroupBox);

  // 계산 제어 영역
  QGroupBox* controlGroupBox = new QGroupBox("최적화 제어");
  QVBoxLayout* controlLayout = new QVBoxLayout(controlGroupBox);

  calculateButton = new QPushButton("경로 계산 (Held-Karp)");
  resetButton = new QPushButton("초기화");
  cropDataButton = new QPushButton("감지된 참외 데이터 사용");
  controlLayout->addWidget(cropDataButton);
  controlLayout->addWidget(calculateButton);
  controlLayout->addWidget(resetButton);

  leftLayout->addWidget(controlGroupBox);

  // 매니퓰레이터 위치 설정 영역 추가
  QGroupBox* manipulatorGroupBox = new QGroupBox("그리퍼 시작 위치");
  QVBoxLayout* manipulatorLayout = new QVBoxLayout(manipulatorGroupBox);

  QLabel* manipulatorLabel = new QLabel("현재 위치: (0, 0, 0) - 그리퍼 원점");
  manipulatorLayout->addWidget(manipulatorLabel);

  // 좌표계 설명 추가 (시각화 회전 설명 포함)
  QLabel* coordExplainLabel = new QLabel(
    "실제 그리퍼 좌표계:\n"
    "• X축: 전진(+) / 후진(-)\n"
    "• Y축: 좌측(+) / 우측(-)\n"
    "• Z축: 상승(+) / 하강(-)\n"
    "\n"
    "※ 시각화는 90도 회전되어 표시\n"
    "※ 실제 데이터는 정확히 전송됨");
  coordExplainLabel->setStyleSheet("QLabel { font-size: 9pt; color: gray; }");
  manipulatorLayout->addWidget(coordExplainLabel);

  leftLayout->addWidget(manipulatorGroupBox);

  // 결과 표시 영역
  QGroupBox* resultGroupBox = new QGroupBox("계산 결과");
  QVBoxLayout* resultLayout = new QVBoxLayout(resultGroupBox);
  resultLabel = new QLabel("아직 계산되지 않음");
  resultLabel->setWordWrap(true);
  resultLayout->addWidget(resultLabel);

  leftLayout->addWidget(resultGroupBox);

  // 상태 표시 영역
  statusLabel = new QLabel("ROS2 통신 대기 중...");
  leftLayout->addWidget(statusLabel);

  leftLayout->addStretch();

  // 오른쪽 패널 (3D 시각화)
  QGroupBox* visualGroupBox = new QGroupBox("3D 시각화 (현실 배치와 일치)");
  QVBoxLayout* visualLayout = new QVBoxLayout(visualGroupBox);

  scatter3D = new Q3DScatter();
  QWidget* container = QWidget::createWindowContainer(scatter3D);
  container->setMinimumSize(300, 300);
  visualLayout->addWidget(container);

  // 3D 시각화 설정
  scatter3D->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
  scatter3D->setShadowQuality(QAbstract3DGraph::ShadowQualitySoftLow);
  scatter3D->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRight);

  // 축 설정 (시각화용 90도 회전 - 실제 Y를 X축에, 실제 X를 Y축에 표시)
  scatter3D->axisX()->setTitle("Y (좌/우)");      // 시각화 X축 = 실제 Y축
  scatter3D->axisY()->setTitle("X (전진/후진)");   // 시각화 Y축 = 실제 X축
  scatter3D->axisZ()->setTitle("Z (상/하)");      // Z축은 동일
  scatter3D->axisX()->setTitleVisible(true);
  scatter3D->axisY()->setTitleVisible(true);
  scatter3D->axisZ()->setTitleVisible(true);

  // 선택 및 카메라 설정
  scatter3D->setSelectionMode(QAbstract3DGraph::SelectionNone);
  scatter3D->scene()->activeCamera()->setCameraPosition(45, 45, 200);
  scatter3D->scene()->activeCamera()->setZoomLevel(85);
  scatter3D->setAspectRatio(1.5f);

  // 시각화 범위 설정 (90도 회전 적용 + X범위 확대)
  scatter3D->axisX()->setRange(-0.3, 0.3);   // 시각화 X축 = 실제 Y (좌/우)
  scatter3D->axisY()->setRange(0.5, 1.0);    // 시각화 Y축 = 실제 X (전진) - 범위 변경
  scatter3D->axisZ()->setRange(-0.2, 0.4);   // Z: 상/하

  // 참외 위치 시리즈 설정
  pointSeries = new QScatter3DSeries;
  pointSeries->setItemSize(0.15f);
  pointSeries->setMesh(QAbstract3DSeries::MeshSphere);
  pointSeries->setBaseColor(QColor(255, 165, 0));  // 참외 색상 (주황색)
  scatter3D->addSeries(pointSeries);

  // 경로 시리즈 설정
  pathSeries = new QScatter3DSeries;
  pathSeries->setItemSize(0.05f);
  pathSeries->setMesh(QAbstract3DSeries::MeshPoint);
  pathSeries->setBaseColor(QColor(Qt::red));  // 경로는 빨간색으로 표시
  scatter3D->addSeries(pathSeries);

  // 경로 순서 표시 시리즈 설정
  orderSeries = new QScatter3DSeries;
  orderSeries->setItemSize(0.2f);
  orderSeries->setMesh(QAbstract3DSeries::MeshCube);  // 큐브 모양으로 표시
  scatter3D->addSeries(orderSeries);

  // 그리퍼 위치 시리즈 설정
  manipulatorSeries = new QScatter3DSeries;
  manipulatorSeries->setItemSize(0.3f);
  manipulatorSeries->setMesh(QAbstract3DSeries::MeshMinimal);
  manipulatorSeries->setBaseColor(QColor(0, 0, 255));  // 파란색
  scatter3D->addSeries(manipulatorSeries);

  // 레이아웃 추가
  mainLayout->addLayout(leftLayout, 1);
  mainLayout->addWidget(visualGroupBox, 2);

  publishButton = new QPushButton("경로 ROS2 발행");
  controlLayout->addWidget(publishButton);
}

void MainWindow::resetAll() {
  // 모든 데이터 초기화
  coordTable->setRowCount(0);
  points.clear();
  optimalPath.clear();
  minCost = 0;
  resultLabel->setText("아직 계산되지 않음");
  statusLabel->setText("ROS2 통신 대기 중...");

  // 기본 점 추가
  addDefaultPoints();

  // 시각화 업데이트
  updateVisualization();
}

void MainWindow::setupConnections() {
  connect(addPointButton, &QPushButton::clicked, this, &MainWindow::addPoint);
  connect(removePointButton, &QPushButton::clicked, this, &MainWindow::removePoint);
  connect(randomizeButton, &QPushButton::clicked, this, &MainWindow::randomizePoints);
  connect(calculateButton, &QPushButton::clicked, this, &MainWindow::calculateOptimalPath);
  connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetAll);
  connect(cropDataButton, &QPushButton::clicked, this, &MainWindow::requestCropData);

  // 테이블 데이터 변경 감지
  connect(coordTable, &QTableWidget::cellChanged, this, &MainWindow::onTableDataChanged);
  connect(publishButton, &QPushButton::clicked, this, &MainWindow::publishOptimalPath);
}

// 참외 데이터 요청 핸들러 (버튼 클릭 시)
void MainWindow::requestCropData() {
  // 이미 수신된 가장 최근 참외 데이터 처리
  processCropData();
}

// ROS2에서 감지된 참외 데이터 처리 (이미 그리퍼 좌표계로 변환됨)
void MainWindow::processCropData() {
  // QNode에서 참외 데이터 가져오기 (calibration에서 변환된 그리퍼 좌표계 데이터)
  const vision_msgs::msg::DetectedCropArray& crop_data = qnode->getCropData();

  if (crop_data.total_objects == 0) {
    QMessageBox::information(this, "정보", "감지된 참외가 없습니다.");
    return;
  }

  isInitializing = true;  // 테이블 업데이트 중 이벤트 발생 방지

  // 기존 데이터 지우기
  coordTable->setRowCount(0);
  points.clear();

  // 감지된 참외 데이터로 테이블과 점 목록 업데이트 (이미 그리퍼 좌표계)
  for (size_t i = 0; i < crop_data.objects.size(); ++i) {
    const auto& crop = crop_data.objects[i];

    int row = coordTable->rowCount();
    coordTable->insertRow(row);

    // 이름은 "참외" + ID로 설정
    QString name = QString("참외%1").arg(crop.id);

    // 좌표는 이미 그리퍼 좌표계로 변환되어 있음 (m 단위)
    // X: 전진/후진, Y: 좌/우, Z: 상/하

    // 테이블 항목 추가 (실제 좌표 그대로)
    coordTable->setItem(row, 0, new QTableWidgetItem(name));
    coordTable->setItem(row, 1, new QTableWidgetItem(QString::number(crop.x, 'f', 3)));  // X (전진)
    coordTable->setItem(row, 2, new QTableWidgetItem(QString::number(crop.y, 'f', 3)));  // Y (좌우)
    coordTable->setItem(row, 3, new QTableWidgetItem(QString::number(crop.z, 'f', 3)));  // Z (상하)

    // points 배열에 추가 (그리퍼 좌표계 - 실제 데이터)
    points.append(Point3D(crop.x, crop.y, crop.z, name));
  }

  isInitializing = false;

  // 경로 초기화
  optimalPath.clear();
  minCost = 0;
  resultLabel->setText("감지된 참외 데이터 로드됨 (그리퍼 좌표계). 경로 계산 필요.");

  // 상태 업데이트
  statusLabel->setText(QString("감지된 참외 수: %1 (그리퍼 좌표계)").arg(crop_data.total_objects));

  // 시각화 업데이트
  updateVisualization();
}

// 테이블 데이터 변경 감지 함수
void MainWindow::onTableDataChanged(int row, int column) {
  // 초기화 중에는 무시
  if (isInitializing) return;

  // 이름 변경이 아닌 좌표 변경일 경우에만 처리
  if (column >= 1 && column <= 3 && row < points.size()) {
    // 변경된 값 읽기
    bool ok;
    float value = coordTable->item(row, column)->text().toFloat(&ok);

    // 변환 실패 시 0으로 설정
    if (!ok) {
      value = 0.0f;
      coordTable->item(row, column)->setText("0.0");
    }

    // points 배열 업데이트 (그리퍼 좌표계 - 실제 데이터)
    if (column == 1)
      points[row].x = value;      // X: 전진/후진
    else if (column == 2)
      points[row].y = value;      // Y: 좌/우
    else if (column == 3)
      points[row].z = value;      // Z: 상/하

    // 시각화 업데이트
    updateVisualization();
  } else if (column == 0 && row < points.size()) {
    // 이름 변경
    points[row].name = coordTable->item(row, 0)->text();
  }
}

void MainWindow::addPoint() {
  int row = coordTable->rowCount();
  coordTable->insertRow(row);

  QString name = QString("참외%1").arg(row + 1);
  coordTable->setItem(row, 0, new QTableWidgetItem(name));
  coordTable->setItem(row, 1, new QTableWidgetItem("0.0"));  // X = 0 (전진/후진)
  coordTable->setItem(row, 2, new QTableWidgetItem("0.0"));  // Y = 0 (좌/우)
  coordTable->setItem(row, 3, new QTableWidgetItem("0.0"));  // Z = 0 (상/하)

  Point3D newPoint(0, 0, 0, name);
  points.append(newPoint);

  updateVisualization();
}

void MainWindow::removePoint() {
  QList<QTableWidgetItem*> selectedItems = coordTable->selectedItems();
  if (selectedItems.isEmpty()) {
    QMessageBox::warning(this, "경고", "제거할 행을 선택하세요.");
    return;
  }

  int row = coordTable->row(selectedItems.first());
  coordTable->removeRow(row);

  // points 배열에서도 제거
  if (row < points.size()) {
    points.remove(row);
  }

  // 경로가 있으면 초기화
  optimalPath.clear();

  updateVisualization();
}

// 랜덤 좌표 생성 함수 (실제 X범위 0.5~1.0으로 수정)
void MainWindow::randomizePoints() {
  isInitializing = true;

  std::random_device rd;
  std::mt19937 gen(rd());

  // 실제 그리퍼 좌표계 범위 (X범위 변경)
  std::uniform_real_distribution<float> distX(0.5f, 1.0f);     // X: 전진 범위 (0.5~1.0m)
  std::uniform_real_distribution<float> distY(-0.2f, 0.2f);    // Y: 좌/우 범위
  std::uniform_real_distribution<float> distZ(-0.1f, 0.3f);    // Z: 상/하 범위

  // 기존의 모든 행에 대해 랜덤 좌표 설정
  for (int row = 0; row < coordTable->rowCount(); ++row) {
    float x = distX(gen);  // 실제 X (전진) - 0.5~1.0m 범위
    float y = distY(gen);  // 실제 Y (좌/우)
    float z = distZ(gen);  // 실제 Z (상/하)

    // 테이블 업데이트 (실제 좌표 그대로 표시)
    coordTable->item(row, 1)->setText(QString::number(x, 'f', 3));
    coordTable->item(row, 2)->setText(QString::number(y, 'f', 3));
    coordTable->item(row, 3)->setText(QString::number(z, 'f', 3));

    // points 배열 업데이트 (실제 데이터 저장 - Master로 전송될 정확한 값)
    if (row < points.size()) {
      points[row].x = x;  // 실제 X (전진)
      points[row].y = y;  // 실제 Y (좌/우)
      points[row].z = z;  // 실제 Z (상/하)
    }
  }

  isInitializing = false;

  // 경로가 있으면 초기화
  optimalPath.clear();

  // 시각화 업데이트 (90도 회전 적용)
  updateVisualization();
}

void MainWindow::updateVisualization() {
  // 시각화 범위 설정 (90도 회전 + X범위 확대)
  scatter3D->axisX()->setRange(-0.3, 0.3);   // 시각화 X축 = 실제 Y (좌/우)
  scatter3D->axisY()->setRange(0.5, 1.0);    // 시각화 Y축 = 실제 X (전진) - 범위 변경
  scatter3D->axisZ()->setRange(-0.2, 0.4);   // Z: 상/하

  // 점 데이터 업데이트 (90도 회전: 실제 XYZ → 시각화 YXZ)
  QScatterDataArray* dataArray = new QScatterDataArray;
  dataArray->resize(points.size());

  for (int i = 0; i < points.size(); ++i) {
    // 시각화만 90도 회전: (실제X, 실제Y, 실제Z) → (실제Y, 실제X, 실제Z)
    (*dataArray)[i].setPosition(QVector3D(points[i].y, points[i].x, points[i].z));
    //                                    ↑시각화X   ↑시각화Y   ↑시각화Z
    //                                    (실제Y)    (실제X)    (실제Z)
  }
  // 점 시리즈 업데이트
  pointSeries->dataProxy()->resetArray(dataArray);

  // 그리퍼 위치 시각화 (90도 회전 적용)
  QtDataVisualization::QScatterDataArray* manipulatorArray = new QtDataVisualization::QScatterDataArray;
  manipulatorArray->resize(1);
  (*manipulatorArray)[0].setPosition(QVector3D(manipulatorPosition.y, manipulatorPosition.x, manipulatorPosition.z));
  manipulatorSeries->dataProxy()->resetArray(manipulatorArray);

  // 경로가 있을 경우 경로 데이터 업데이트
  if (!optimalPath.isEmpty()) {
    updatePathVisualization();
  } else {
    // 경로가 없으면 경로 시리즈 초기화
    pathSeries->dataProxy()->resetArray(new QScatterDataArray);
    orderSeries->dataProxy()->resetArray(new QScatterDataArray);
  }
}

void MainWindow::updatePathVisualization() {
  // 경로가 없으면 초기화
  if (optimalPath.isEmpty()) {
    pathSeries->dataProxy()->resetArray(new QScatterDataArray);
    orderSeries->dataProxy()->resetArray(new QScatterDataArray);
    return;
  }

  // 경로 선 그리기
  QScatterDataArray* pathArray = new QScatterDataArray;
  int pathLength = optimalPath.size();
  int pointsPerSegment = 100;
  int totalPoints = (pathLength - 1) * pointsPerSegment;
  pathArray->resize(totalPoints);

  int arrayIndex = 0;

  // 각 세그먼트를 여러 점으로 보간하여 선으로 표현
  for (int i = 0; i < pathLength - 1; ++i) {
    QVector3D startPos, endPos;

    // 시작점 결정 (90도 회전 적용)
    if (optimalPath[i] == -1) {
      // 그리퍼 위치 (원점)
      startPos = QVector3D(manipulatorPosition.y, manipulatorPosition.x, manipulatorPosition.z);
    } else {
      // 참외 위치 (90도 회전 적용)
      startPos = QVector3D(points[optimalPath[i]].y, points[optimalPath[i]].x, points[optimalPath[i]].z);
    }

    // 끝점 결정 (90도 회전 적용)
    if (optimalPath[i + 1] == -1) {
      // 그리퍼 위치 (원점)
      endPos = QVector3D(manipulatorPosition.y, manipulatorPosition.x, manipulatorPosition.z);
    } else {
      // 참외 위치 (90도 회전 적용)
      endPos = QVector3D(points[optimalPath[i + 1]].y, points[optimalPath[i + 1]].x, points[optimalPath[i + 1]].z);
    }

    // 선분을 여러 점으로 보간
    for (int j = 0; j < pointsPerSegment; ++j) {
      float t = static_cast<float>(j) / pointsPerSegment;
      float x = startPos.x() * (1 - t) + endPos.x() * t;  // 시각화 X (실제 Y)
      float y = startPos.y() * (1 - t) + endPos.y() * t;  // 시각화 Y (실제 X)
      float z = startPos.z() * (1 - t) + endPos.z() * t;  // 시각화 Z (실제 Z)

      (*pathArray)[arrayIndex++].setPosition(QVector3D(x, y, z));
    }
  }

  // 경로 시리즈 업데이트
  pathSeries->dataProxy()->resetArray(pathArray);
  pathSeries->setItemSize(0.05f);
  pathSeries->setBaseColor(QColor(Qt::red));  // 경로는 빨간색
}

// 최적 경로 계산 함수 (그리퍼 좌표계에서 거리 계산)
void MainWindow::calculateOptimalPath() {
  int n = points.size();
  if (n == 0) {
    QMessageBox::warning(this, "경고", "최소 1개 이상의 참외가 필요합니다.");
    return;
  }

  // 결과 초기화
  optimalPath.clear();
  minCost = 0;

  // 그리퍼와 모든 참외 간의 거리 행렬 계산 (그리퍼 좌표계에서)
  std::vector<std::vector<double>> dist(n + 1, std::vector<double>(n + 1, 0));

  // 그리퍼 위치는 인덱스 0으로 처리
  for (int i = 0; i <= n; ++i) {
    for (int j = 0; j <= n; ++j) {
      if (i == j) {
        dist[i][j] = 0;  // 자기 자신까지의 거리는 0
      } else if (i == 0) {
        // 그리퍼에서 참외까지의 거리 (그리퍼 좌표계에서)
        double dx = manipulatorPosition.x - points[j - 1].x;
        double dy = manipulatorPosition.y - points[j - 1].y;
        double dz = manipulatorPosition.z - points[j - 1].z;
        dist[i][j] = std::sqrt(dx * dx + dy * dy + dz * dz);
      } else if (j == 0) {
        // 참외에서 그리퍼까지의 거리 (그리퍼 좌표계에서)
        double dx = points[i - 1].x - manipulatorPosition.x;
        double dy = points[i - 1].y - manipulatorPosition.y;
        double dz = points[i - 1].z - manipulatorPosition.z;
        dist[i][j] = std::sqrt(dx * dx + dy * dy + dz * dz);
      } else {
        // 참외 간의 거리 (그리퍼 좌표계에서)
        double dx = points[i - 1].x - points[j - 1].x;
        double dy = points[i - 1].y - points[j - 1].y;
        double dz = points[i - 1].z - points[j - 1].z;
        dist[i][j] = std::sqrt(dx * dx + dy * dy + dz * dz);
      }
    }
  }

  // 참외가 적을 경우(11개 이하) 모든 순열을 시도하는 방식 사용
  if (n <= 11) {
    std::vector<int> indices;
    for (int i = 1; i <= n; ++i) {  // 1부터 시작 (0은 그리퍼 위치)
      indices.push_back(i);
    }

    std::vector<int> bestPath;
    double bestCost = std::numeric_limits<double>::max();

    // 모든 순열 시도
    do {
      double currentCost = dist[0][indices[0]];  // 그리퍼에서 첫 번째 참외까지

      for (int i = 0; i < indices.size() - 1; ++i) {
        currentCost += dist[indices[i]][indices[i + 1]];
      }

      currentCost += dist[indices.back()][0];  // 마지막 참외에서 그리퍼로

      if (currentCost < bestCost) {
        bestCost = currentCost;
        bestPath = indices;
      }

    } while (std::next_permutation(indices.begin(), indices.end()));

    // 최적 경로 구성 (그리퍼 위치에서 시작하여 다시 돌아옴)
    optimalPath.clear();
    optimalPath.push_back(-1);  // 그리퍼 위치 (-1로 표시)

    for (int idx : bestPath) {
      optimalPath.push_back(idx - 1);  // 실제 points 배열 인덱스로 변환
    }

    optimalPath.push_back(-1);  // 그리퍼 위치로 돌아오기
    minCost = bestCost;
  } else {
    // 많은 참외가 있을 경우 Held-Karp 알고리즘 사용
    if (n > 20) {
      QMessageBox::warning(this, "경고", "계산 효율성을 위해 최대 20개 참외까지 지원합니다.");
      return;
    }

    // Held-Karp 알고리즘 (비트마스크를 사용한 동적 계획법)
    std::unordered_map<std::pair<int, int>, double, StateHash> dp;
    std::unordered_map<std::pair<int, int>, int, StateHash> parent;

    // 참외 개수 + 그리퍼 = n + 1개 도시
    int totalCities = n + 1;

    // 초기 상태: 0번 점(그리퍼)만 방문
    for (int j = 1; j <= n; ++j) {  // 참외들 (1부터 n까지)
      dp[{1 | (1 << j), j}] = dist[0][j];
      parent[{1 | (1 << j), j}] = 0;
    }

    // 모든 부분집합에 대해 계산
    int allVisited = (1 << totalCities) - 1;
    for (int mask = 3; mask <= allVisited; ++mask) {
      // 0번(그리퍼)은 항상 시작점이므로 포함되어야 함
      if (!(mask & 1)) continue;

      for (int j = 1; j <= n; ++j) {  // 참외들만 확인
        // j가 mask에 포함되어 있는지 확인
        if (!(mask & (1 << j))) continue;

        // j 이전의 마지막 도시를 결정
        int prevMask = mask & ~(1 << j);
        double minCost = std::numeric_limits<double>::max();
        int minPrev = -1;

        for (int k = 0; k <= n; ++k) {  // 그리퍼 포함 모든 위치
          if (!(prevMask & (1 << k))) continue;
          if (k == j) continue;

          auto it = dp.find({prevMask, k});
          if (it == dp.end()) continue;

          double cost = it->second + dist[k][j];
          if (cost < minCost) {
            minCost = cost;
            minPrev = k;
          }
        }

        if (minPrev != -1) {
          dp[{mask, j}] = minCost;
          parent[{mask, j}] = minPrev;
        }
      }
    }

    // 최종 경로 비용 계산 (모든 참외 방문 후 그리퍼로 돌아오기)
    minCost = std::numeric_limits<double>::max();
    int lastCity = -1;

    for (int j = 1; j <= n; ++j) {  // 참외들만 확인
      auto it = dp.find({allVisited, j});
      if (it == dp.end()) continue;

      double cost = it->second + dist[j][0];  // j에서 그리퍼(0)로
      if (cost < minCost) {
        minCost = cost;
        lastCity = j;
      }
    }

    if (lastCity == -1) {
      QMessageBox::warning(this, "경고", "경로를 찾을 수 없습니다.");
      return;
    }

    // 경로 재구성
    optimalPath.clear();
    optimalPath.push_back(-1);  // 그리퍼 시작

    std::vector<int> pathSequence;
    int mask = allVisited;
    int currentCity = lastCity;

    pathSequence.push_back(currentCity);

    // 역추적으로 경로 복원
    while (currentCity != 0) {
      auto it = parent.find({mask, currentCity});
      if (it == parent.end()) break;

      int nextMask = mask & ~(1 << currentCity);
      int nextCity = it->second;

      if (nextCity != 0) {  // 그리퍼가 아닌 경우만
        pathSequence.push_back(nextCity);
      }

      mask = nextMask;
      currentCity = nextCity;
    }

    // 경로를 시작점부터 순서대로 정렬
    std::reverse(pathSequence.begin(), pathSequence.end());

    for (int city : pathSequence) {
      optimalPath.push_back(city - 1);  // 실제 points 배열 인덱스로 변환
    }

    optimalPath.push_back(-1);  // 그리퍼로 돌아오기
  }

  // 결과 표시 (그리퍼 좌표계 기준)
  QString resultText = QString("최적 경로 비용: %1m\n경로: %2").arg(minCost, 0, 'f', 3).arg(manipulatorPosition.name);

  for (int i = 1; i < optimalPath.size() - 1; ++i) {
    int idx = optimalPath[i];
    resultText += " → " + points[idx].name;
  }

  resultText += " → " + manipulatorPosition.name;
  resultLabel->setText(resultText);

  // 시각화 업데이트
  updatePathVisualization();
  publishOptimalPath();
}

void MainWindow::addDefaultPoints() {
  isInitializing = true;

  // 테이블 초기화
  coordTable->setRowCount(0);
  points.clear();

  // 랜덤 좌표 생성을 위한 설정
  std::random_device rd;
  std::mt19937 gen(rd());

  // 실제 그리퍼 좌표계 범위 (X범위 변경)
  std::uniform_real_distribution<float> distX(0.5f, 1.0f);     // X: 전진 (0.5~1.0m)
  std::uniform_real_distribution<float> distY(-0.15f, 0.15f);  // Y: 좌/우
  std::uniform_real_distribution<float> distZ(0.0f, 0.2f);     // Z: 상/하 범위 (양수 위주)

  // 6개의 참외 생성 (그리퍼 좌표계 랜덤 좌표)
  const int numPoints = 6;

  for (int i = 0; i < numPoints; ++i) {
    int row = coordTable->rowCount();
    coordTable->insertRow(row);

    // 실제 그리퍼 좌표계에 맞는 랜덤 좌표 생성
    float x = distX(gen);  // 실제 X (전진) - 0.5~1.0m
    float y = distY(gen);  // 실제 Y (좌/우)
    float z = distZ(gen);  // 실제 Z (상/하)

    QString name = QString("참외%1").arg(i + 1);
    coordTable->setItem(row, 0, new QTableWidgetItem(name));
    coordTable->setItem(row, 1, new QTableWidgetItem(QString::number(x, 'f', 3)));
    coordTable->setItem(row, 2, new QTableWidgetItem(QString::number(y, 'f', 3)));
    coordTable->setItem(row, 3, new QTableWidgetItem(QString::number(z, 'f', 3)));

    // 실제 데이터 저장 (Master로 전송될 정확한 값)
    points.append(Point3D(x, y, z, name));
  }

  isInitializing = false;
}

// 최적 경로 ROS2 발행 함수 (그리퍼 좌표계 기준)
void MainWindow::publishOptimalPath() {
  // 경로가 없으면 발행하지 않음
  if (optimalPath.isEmpty()) {
    statusLabel->setText("발행할 경로가 없습니다. 먼저 경로를 계산하세요.");
    return;
  }

  // optimalPath에서 그리퍼 위치 제외하고 실제 참외만 포함된 경로 생성
  QVector<int> harvestPath;

  // 그리퍼 시작/종료 위치 제외하고 중간 참외들만 수집
  for (int i = 1; i < optimalPath.size() - 1; ++i) {
    if (optimalPath[i] != -1) {
      harvestPath.append(optimalPath[i]);
    }
  }

  // QNode를 통해 수확 순서 발행 (그리퍼 시작 위치 정보 포함)
  // 실제 좌표값 그대로 전송 (시각화 회전과 무관)
  qnode->publishHarvestOrder(harvestPath, points, manipulatorPosition);

  // 상태 업데이트
  statusLabel->setText("수확 순서가 ROS2 토픽 '/harvest_ordering/result2'에 발행되었습니다 (그리퍼 좌표계).");
}