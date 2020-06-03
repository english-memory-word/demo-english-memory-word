#include <QRegExp>
#include <QRegExpValidator>
#include <QValidator>

#include "regex.h"

bool IsEng(QString input){
  // 正则表达式，判断输入内容是中文/英文
  QRegExp reg("^[a-zA-Z\\s,.!]+$");
  QRegExpValidator vaildator(reg,0);
  int pos=0;
  QValidator::State res=vaildator.validate(input,pos);

  if(QValidator::Acceptable==res){
    // 输入内容是英文
    return true;
  }else{
    // 输入内容是中文
    return false;
  }
}
