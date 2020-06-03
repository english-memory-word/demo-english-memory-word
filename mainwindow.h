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

  const int TOTAL_QUESTION_NUM=33;
  enum req_type{TRANSLATE,PRACTICE} req;

  NetworkSupport *networkObj;

  QList<int> id_list;
  QList<QString> question_list;
  QList<QString> word_list;
  QList<QChar> my_answer_list;
  QList<QChar> solution_list;
  QString difficulty;

public slots:
  void requestFail(QString str); //发送“失败信号”时，触发该方法
  void requestSuccess(QString str);//发送“成功信号”时，触发该方法
private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
