#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QList>
#include "networksupport.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  QTimer * trans_timer;
  QTimer * practice_timer;
  NetworkSupport *networkObj;
  QList<char> answer_list;
  QList<char> my_answer_list;
  QList<QString> question_list;
  QList<QString> solution_list;

  int question_index;

public slots:
    void requestFail(QString str); //发送“失败信号”时，触发该方法
    void requestSuccess(QString str);//发送“成功信号”时，触发该方法
private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
