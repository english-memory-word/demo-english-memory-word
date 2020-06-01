#include <QToolButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QListWidget>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextToSpeech>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networksupport.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  /* 工具栏 */
  // 翻译按钮
  connect(ui->transBtn,&QToolButton::clicked,[=](){
    ui->stackedWidget->setCurrentIndex(0);
    ui->transBtn->setChecked(true);
    ui->dictBtn->setChecked(false);
    ui->practiceBtn->setChecked(false);
    ui->transBtn->setIcon(QIcon(":/icon/trans_selected.png"));
    ui->dictBtn->setIcon(QIcon(":/icon/dict.png"));
    ui->practiceBtn->setIcon(QIcon(":/icon/practice.png"));
    ui->favorBtn->setChecked(false);
    ui->favorBtn->setIcon(QIcon(":/icon/favor.png"));
    ui->transInput->setPlainText("");
    ui->transResult->setText("");
    networkObj->dataStr="";
    trans_timer->stop();
    practice_timer->stop();
  });

  //词典按钮
  connect(ui->dictBtn,&QToolButton::clicked,[=](){
    ui->stackedWidget->setCurrentIndex(1);
    ui->dictBtn->setChecked(true);
    ui->transBtn->setChecked(false);
    ui->practiceBtn->setChecked(false);
    ui->transBtn->setIcon(QIcon(":/icon/trans.png"));
    ui->dictBtn->setIcon(QIcon(":/icon/dict_selected.png"));
    ui->practiceBtn->setIcon(QIcon(":/icon/practice.png"));
    trans_timer->stop();
    practice_timer->stop();
  });


  //练习按钮
  connect(ui->practiceBtn,&QToolButton::clicked,[=](){
    ui->stackedWidget->setCurrentIndex(2);
    ui->practiceBtn->setChecked(true);
    ui->transBtn->setChecked(false);
    ui->dictBtn->setChecked(false);
    ui->transBtn->setIcon(QIcon(":/icon/trans.png"));
    ui->dictBtn->setIcon(QIcon(":/icon/dict.png"));
    ui->practiceBtn->setIcon(QIcon(":/icon/practice_selected.png"));
    trans_timer->stop();
    practice_timer->stop();
  });

  /* 翻译界面 */
  //文本内容改变取消收藏
  connect(ui->transInput,&QPlainTextEdit::textChanged,[=](){
    ui->favorBtn->setChecked(false);
    ui->favorBtn->setIcon(QIcon(":/icon/favor.png"));
  });
  //查找按钮
  connect(ui->getTransBtn,&QToolButton::clicked,[=](){
    networkObj->dataStr="请稍候...";
    QString str="http://test.cpp-homework.su29029.xyz/translate?query="+ui->transInput->toPlainText();
    QUrl url(str);
    qDebug()<<url;
    networkObj->get(url); //发送get请求
    trans_timer->start(500);
  });

  //播音按钮
  connect(ui->inputReadingBtn,&QToolButton::clicked,[=](){
    QTextToSpeech *tts=new QTextToSpeech;
    if(tts->state()==QTextToSpeech::Ready)
      tts->say(ui->transInput->toPlainText());
    else
      QMessageBox::warning(this,"请检查tts语音引擎","语音播放失败，您的电脑可能未安装tts语音引擎，请自行检查并安装！");
  });

  connect(ui->transReadingBtn,&QToolButton::clicked,[=](){
    QTextToSpeech *tts=new QTextToSpeech;
    if(tts->state()==QTextToSpeech::Ready)
      tts->say(ui->transResult->text());
    else
      QMessageBox::warning(this,"请检查tts语音引擎","语音播放失败，您的电脑可能未安装tts语音引擎，请自行检查并安装！");
  });

  // 收藏按钮
  connect(ui->favorBtn,&QToolButton::clicked,[=](){
    if(ui->favorBtn->isChecked() && ui->transInput->toPlainText()!=""){
      ui->favorBtn->setIcon(QIcon(":/icon/favor_selected.png"));
      QString result=ui->transResult->text();
      QString input=ui->transInput->toPlainText();

      // 正则表达式，判断输入内容是中文/英文
      QRegExp reg("^[a-zA-Z\\s]+$");
      QRegExpValidator vaildator(reg,0);
      int pos=0;
      QValidator::State res=vaildator.validate(input,pos);

      if(QValidator::Acceptable==res){
        // 输入内容是英文
        for(int i=input.length();i<12;++i)
          input+=" ";
        ui->wordList->addItem(input+result);
      }else{
        // 输入内容是中文
        for(int i=2*result.length();i<12;++i)
          result+=" ";
        ui->wordList->addItem(result+input);
      }
    }else{
      ui->favorBtn->setIcon(QIcon(":/icon/favor.png"));
      ui->favorBtn->setChecked(false);
    }
  });

  /* 词典界面 */
  //删除按钮
  connect(ui->delBtn,&QToolButton::clicked,[=]() {
    if(ui->delText->toPlainText()!=""){
      int row=0;
      while(row<ui->wordList->count()){
        if(ui->wordList->item(row)->text().contains(ui->delText->toPlainText())){
          QMessageBox::StandardButton result= QMessageBox::question(this,"删除","确定删除："+ui->wordList->item(row)->text()+"？");
          qDebug()<<result;
          if(result==QMessageBox::Yes)
            ui->wordList->takeItem(row);
          else
            ++row;
        }else{
          ++row;
        }
      }
      ui->delText->setPlainText("");
    }
  });

  /* 练习开始界面 */
  // Slider与SpinBox联动
  connect(ui->practiceNumSpinBox,SIGNAL(valueChanged(int)),ui->practiceNumSlider,SLOT(setValue(int)));
  connect(ui->practiceNumSlider,SIGNAL(valueChanged(int)),ui->practiceNumSpinBox,SLOT(setValue(int)));

  // 开始练习按钮
  connect(ui->practiceStartBtn,&QPushButton::clicked,[=](){
    ui->totalNum->setNum(ui->practiceNumSpinBox->value());
    ui->currentNum->setNum(1);
    ui->stackedWidget->setCurrentIndex(3);
    ui->aRadio->setChecked(true);
    practice_timer->start(500);
    question_index=0;
    my_answer_list.clear();
    for(int i=0;i<ui->practiceNumSpinBox->value();++i){
      answer_list.append(0);
      question_list.append("");
      solution_list.append("");
    }
    ui->practiceNextBtn->setEnabled(false);
  });

  /* 练习界面 */
  // 下一题按钮
  connect(ui->practiceNextBtn,&QPushButton::clicked,[=](){
    if(ui->aRadio->isChecked())
      my_answer_list.append('a');
    else if(ui->bRadio->isChecked())
      my_answer_list.append('b');
    else if(ui->cRadio->isChecked())
      my_answer_list.append('c');

    ui->currentNum->setNum(ui->currentNum->text().toInt()+1);
    ui->aRadio->setChecked(true);

    // 练习结束
    if(ui->currentNum->text().toInt()>ui->totalNum->text().toInt()){
      ui->stackedWidget->setCurrentIndex(4);
      practice_timer->stop();
      ui->totalNumEnd->setNum(ui->practiceNumSpinBox->value());
      int error_num=0;
      for(int i=0;i<question_index;++i)
        if(answer_list[i]!=my_answer_list[i])
          ++error_num;
      ui->errorNumEnd->setNum(error_num);
      ui->correctRateEnd->setText(
            QString::number((ui->practiceNumSpinBox->value()-error_num)*100/ui->practiceNumSpinBox->value())+"%");
      ui->questionStatisticsList->clear();
      ui->practiceNumSpinBox->setValue(5);

      // 导入题目统计到列表
      for(int i=0;i<=question_index;++i){
        QString question=question_list[i];
        for(int i=question.length();i<16;++i)
          question+=" ";
        QString answer=solution_list[i];
        for(int i=answer.length();i<12;++i)
          answer+=" ";
        if(answer_list[i]==my_answer_list[i])
          ui->questionStatisticsList->addItem(question+answer+"正确");
        else
          ui->questionStatisticsList->addItem(question+answer+"错误");
      }
    }

    ++question_index;
    ui->practiceNextBtn->setEnabled(false);
  });

  /* 练习结束界面 */
  // 返回按钮
  connect(ui->backBtnEnd,&QPushButton::clicked,[=](){
    ui->stackedWidget->setCurrentIndex(2);
  });

  //统计结果按钮
  connect(ui->createResultBtn,&QPushButton::clicked,[=](){
    QString path=QFileDialog::getExistingDirectory(this,"保存到");
    QFile save_file(path+"/我的错题.txt");
    save_file.open(QIODevice::WriteOnly);
    save_file.close();
    if(!save_file.open(QIODevice::WriteOnly)){
      QMessageBox::warning(this,"文件打开错误","文件打开错误，请检查路径是否正确！");
    }else{
      QTextStream stream(&save_file);
      for(int i=0;i<question_index;++i)
        stream<<ui->questionStatisticsList->item(i)->text()<<"\n";
      save_file.close();
    }
  });

  /* 初始化 */
  ui->stackedWidget->setCurrentIndex(0);
  ui->statusbar->addPermanentWidget(
        new QLabel("Copyright © 2020 Designed by Koorye. All rights reserved.            "));

  this->resize(800,600);
  this->setFixedSize(800,600);
  this->setWindowTitle("英语单词小工具Demo");
  this->setWindowIcon(QIcon(":/icon/icon.jpg"));

  // 读取词典文件
  QFile dict(QDir::currentPath()+"/dict.json");
  if(dict.open(QIODevice::ReadOnly)){
    QByteArray data=dict.readAll();

    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data,&json_error));

    if(json_error.error!=QJsonParseError::NoError){
      QMessageBox::warning(this,"词典文件格式错误","检测到词典文件格式错误，将为您重置词典！");
      dict.open(QIODevice::WriteOnly);
      dict.write("");
      dict.close();
    }else{
      QJsonArray array=jsonDoc.array();

      foreach(auto each,array)
        ui->wordList->addItem(each.toString());
    }
  }else{
    QMessageBox::warning(this,"未找到词典文件","未找到词典文件，将为您重新创建词典文件！");
  }
  dict.close();

  // http请求
  networkObj = new NetworkSupport();
  connect(networkObj,SIGNAL(requestSuccessSignal(QString)),this,SLOT(requestSuccess(QString)));
  connect(networkObj,SIGNAL(requestFailSignal(QString)),this,SLOT(requestFail(QString)));

  // 翻译结果同步
  trans_timer=new QTimer(this);
  connect(trans_timer,&QTimer::timeout,[=](){
    qDebug()<<"翻译同步中";
    ui->transResult->setText(networkObj->dataStr.trimmed());
  });

  // 练习题同步
  practice_timer=new QTimer(this);
  connect(practice_timer,&QTimer::timeout,[=](){
    qDebug()<<"练习同步中";
    ui->practice_question->setText("hello");
    ui->aAnswer->setText("你好");
    ui->bAnswer->setText("再见");
    ui->cAnswer->setText("谢谢");
    answer_list[question_index]='a';
    question_list[question_index]="hello";
    solution_list[question_index]="你好";
    ui->practiceNextBtn->setEnabled(true);
  });
}

MainWindow::~MainWindow(){
  // 存储词典
  QFile dict(QDir::currentPath()+"/dict.json");
  if(dict.open(QIODevice::WriteOnly)){
    QJsonArray array;
    int row=0;
    while(row<ui->wordList->count())
      array.append(ui->wordList->item(row++)->text());

    QJsonDocument jsonDoc;
    jsonDoc.setArray(array);
    dict.write(jsonDoc.toJson());
  }else{
    QMessageBox::warning(this,"未找到词典文件","未找到词典文件，将为您重新创建词典文件！");
  }
  dict.close();

  delete ui;
}

void MainWindow::requestFail(QString str){
  qDebug() << "----------requestFail-------------";
  qDebug() << str;
}

void MainWindow::requestSuccess(QString str){
  qDebug() << "----------requestSuccess-------------";
  qDebug() << str;
}
