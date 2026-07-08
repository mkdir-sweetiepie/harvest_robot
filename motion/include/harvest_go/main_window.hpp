/**
 * @file /include/harvest_go/main_window.hpp
 *
 * @brief Qt based gui for harvest_go.
 *
 * @date January 2025
 **/

#ifndef harvest_go_MAIN_WINDOW_H
#define harvest_go_MAIN_WINDOW_H

/*****************************************************************************
** Includes
*****************************************************************************/

#include <QMainWindow>
#include <chrono>
#include <thread>

#include "QIcon"
#include "ui_mainwindow.h"

/*****************************************************************************
** Interface [MainWindow]
*****************************************************************************/

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private:
  Ui::MainWindowDesign* ui;
  void closeEvent(QCloseEvent* event);

 public Q_SLOTS:
  // ========== 비전 시스템 ==========
  void on_Vision_All_On_clicked();
  void on_Vision_All_Off_clicked();
  void on_RealSense_clicked();
  void on_Calibration_clicked();
  void on_YOLO_clicked();
  void on_FoundationPose_clicked();
  void on_TSP_clicked();

  // ========== 매니퓰레이터 시스템 ==========
  void on_Mani_All_On_clicked();
  void on_Mani_All_Off_clicked();
  void on_Motor_clicked();
  void on_Map_clicked();
  void on_Path_clicked();
  void on_Inverse_Sim_clicked();
  void on_Real_Inverse_clicked();
  void on_SAD_Calibration_clicked();
  void on_launch_clicked();

  // ========== 시뮬레이션 & 시각화 ==========
  void on_Gazebo_Launch_clicked();
  void on_RViz_clicked();

  // ========== 전체 시스템 ==========
  void on_System_All_On_clicked();
  void on_System_All_Off_clicked();
};

#endif  // harvest_go_MAIN_WINDOW_H