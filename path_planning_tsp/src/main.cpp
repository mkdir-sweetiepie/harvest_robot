/**
 * @file /src/main.cpp
 *
 * @brief Qt based GUI with ROS2 integration.
 *
 * @date January 2025
 **/

 #include <QApplication>
 #include "../include/tsp/main_window.hpp"
 
 int main(int argc, char** argv)
 {
   QApplication app(argc, argv);
   MainWindow w;
   w.show();
   return app.exec();
 }