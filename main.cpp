#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QFile style(":style.qss");
  if(style.open(QFile::ReadOnly)){
  a.setStyleSheet(QLatin1String(style.readAll()));
  style.close();
  }else{
    qDebug()<<"qss loading failed";
  }

  MainWindow w;
  w.show();
  return a.exec();
}
