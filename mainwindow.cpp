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
#include <QTime>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networksupport.h"
#include "regex.h"

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
    ui->transSymbol->setText("");
    ui->transReadingBtn->setHidden(true);
    networkObj->dataStr="";
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
  });

  /* 翻译界面 */
  //文本内容改变取消收藏
  connect(ui->transInput,&QPlainTextEdit::textChanged,[=](){
    ui->favorBtn->setChecked(false);
    ui->favorBtn->setIcon(QIcon(":/icon/favor.png"));
  });
  //查找按钮
  connect(ui->getTransBtn,&QToolButton::clicked,[=](){
    ui->transResult->setText("请稍候...");
    ui->transReadingBtn->setHidden(true);
    if(IsEng(ui->transInput->toPlainText()))
      ui->transSymbol->setText("请稍候...");
    else
      ui->transSymbol->setText("");

    QString str="http://test.cpp-homework.su29029.xyz/translate?query="+ui->transInput->toPlainText();
    QUrl url(str);
    networkObj->get(url); //发送get请求
    req=TRANSLATE;
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

      if(IsEng(input)){
        for(int i=input.length();i<12;++i)
          input+=" ";
        ui->wordList->addItem(input+result);
      }else{
        for(int i=result.length();i<12;++i)
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
    ui->currentNum->setNum(0);
    ui->aRadio->setChecked(true);
    ui->practiceNextBtn->setEnabled(false);
    ui->practice_question->setText("请稍候...");
    ui->aAnswer->setText("请稍候...");
    ui->bAnswer->setText("请稍候...");
    ui->cAnswer->setText("请稍候...");

    my_answer_list.clear();
    word_list.clear();
    solution_list.clear();
    question_list.clear();
    id_list.clear();

    while(id_list.length()<ui->practiceNumSpinBox->value()){
      int id=qrand() % TOTAL_QUESTION_NUM+1;
      while(id_list.contains(id))
        id=qrand() % TOTAL_QUESTION_NUM+1;
      id_list.append(id);
    }

    qDebug()<<id_list;

    if(ui->easyRadio->isChecked())
      difficulty="easy";
    else if(ui->normalRadio->isChecked())
      difficulty="normal";
    else if(ui->hardRadio->isChecked())
      difficulty="hard";

    QString str="http://test.cpp-homework.su29029.xyz/getProblem?id="
        +QString::number(id_list[ui->currentNum->text().toInt()])+"&difficulty="+difficulty;
    QUrl url(str);
    networkObj->get(url); //发送get请求
    req=PRACTICE;

    ui->stackedWidget->setCurrentIndex(3);
    ui->practiceNextBtn->setEnabled(false);
    ui->practiceNextBtn->setText("请稍候...");
  });

  /* 练习界面 */
  // 下一题按钮
  connect(ui->practiceNextBtn,&QPushButton::clicked,[=](){
    // 填入我的答案
    if(ui->aRadio->isChecked())
      my_answer_list.append('A');
    else if(ui->bRadio->isChecked())
      my_answer_list.append('B');
    else if(ui->cRadio->isChecked())
      my_answer_list.append('C');

    // 切换下一题
    if(ui->currentNum->text().toInt()<ui->totalNum->text().toInt()){
      ui->aRadio->setChecked(true);
      ui->practiceNextBtn->setEnabled(false);
      ui->practiceNextBtn->setText("请稍候...");
      ui->practice_question->setText("请稍候...");
      ui->aAnswer->setText("请稍候...");
      ui->bAnswer->setText("请稍候...");
      ui->cAnswer->setText("请稍候...");

      QString str="http://test.cpp-homework.su29029.xyz/getProblem?id="
          +QString::number(id_list[ui->currentNum->text().toInt()])+"&difficulty="+difficulty;
      QUrl url(str);
      networkObj->get(url); //发送get请求
    }else{
      // 练习结束
      ui->stackedWidget->setCurrentIndex(4);
      ui->totalNumEnd->setNum(ui->practiceNumSpinBox->value());
      int error_num=0;
      for(int i=0;i<question_list.length();++i)
        if(solution_list[i]!=my_answer_list[i])
          ++error_num;
      ui->errorNumEnd->setNum(error_num);
      ui->correctRateEnd->setText(
            QString::number((ui->practiceNumSpinBox->value()-error_num)*100/ui->practiceNumSpinBox->value())+"%");
      ui->questionStatisticsList->clear();
      ui->practiceNumSpinBox->setValue(5);

      // 导入题目统计到列表
      for(int i=0;i<question_list.length();++i){
        QString question=question_list[i];
        QString answer=word_list[i];
        QString str;
        if(IsEng(question)){
          for(int i=question.length();i<16;++i)
            question+=" ";
          for(int i=answer.length()*2;i<16;++i)
            answer+=" ";
          str=question+answer;
        }else{
          for(int i=question.length()*2;i<16;++i)
            question+=" ";
          for(int i=answer.length();i<16;++i)
            answer+=" ";
          str=answer+question;
        }
        if(solution_list[i]==my_answer_list[i])
          ui->questionStatisticsList->addItem(str+"正确");
        else
          ui->questionStatisticsList->addItem(str+"错误");
      }
    }
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
      for(int i=0;i<question_list.length();++i)
        stream<<ui->questionStatisticsList->item(i)->text()<<"\n";
      save_file.close();
    }
  });

  /* 初始化 */
  ui->stackedWidget->setCurrentIndex(0);
  ui->statusbar->addPermanentWidget(
        new QLabel("Copyright © 2020 Designed by Koorye. All rights reserved.            "));
  ui->transReadingBtn->setHidden(true);

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

  QTime time;
  time= QTime::currentTime();
  qsrand(time.msec()+time.second()*1000);
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
  if(req==TRANSLATE){
    ui->transResult->setText("访问失败，请重试");
    ui->transSymbol->setText("");
  }else if(req==PRACTICE){
    QMessageBox::warning(this,"访问失败","访问失败，请重试");
    ui->practiceNextBtn->setEnabled(true);
    ui->practiceNextBtn->setText("下一题！");
  }
}

void MainWindow::requestSuccess(QString res){
  qDebug() << "----------requestSuccess-------------";
  qDebug() << res;
  if(req==TRANSLATE){
    ui->transReadingBtn->setHidden(false);
    QByteArray data=res.toUtf8();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data,&json_error));

    if(json_error.error!=QJsonParseError::NoError){
      // 仅返回结果
      ui->transResult->setText(res.trimmed());
      ui->transSymbol->setText("");
    }else{
      // 返回结果和音标的json
      QJsonObject obj=jsonDoc.object();
      ui->transResult->setText(obj.value("res").toString());
      ui->transSymbol->setText("英："+obj.value("ph_en").toString()
                               +"\n美："+obj.value("ph_am").toString());
    }
  }else if(req==PRACTICE){
    QByteArray data=res.toUtf8();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data,&json_error));

    if(json_error.error!=QJsonParseError::NoError){
      QMessageBox::warning(this,"未知错误","未知错误");
    }else{
      QJsonObject obj=jsonDoc.object();
      ui->practice_question->setText(obj.value("question").toString());

      QByteArray answer_data=obj.value("answer").toString().toUtf8();
      QJsonDocument answer_doc(QJsonDocument::fromJson(answer_data,&json_error));
      if(json_error.error!=QJsonParseError::NoError){
        QMessageBox::warning(this,"解析错误","JSON格式解析错误");
      }else{
        QJsonObject answer=answer_doc.object();

        ui->currentNum->setNum(ui->currentNum->text().toInt()+1);
        ui->aAnswer->setText(answer.value("A").toString());
        ui->bAnswer->setText(answer.value("B").toString());
        ui->cAnswer->setText(answer.value("C").toString());
        question_list.append(obj.value("question").toString());
        solution_list.append(obj.value("solution").toString().front());

        if(obj.value("solution").toString().front()=='A')
          word_list.append(answer.value("A").toString());
        else if(obj.value("solution").toString().front()=='B')
          word_list.append(answer.value("B").toString());
        else
          word_list.append(answer.value("C").toString());
      }
    }
    ui->practiceNextBtn->setEnabled(true);
    ui->practiceNextBtn->setText("下一题！");
  }
}
