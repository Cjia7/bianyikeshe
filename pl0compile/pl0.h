#ifndef PL__H
#define PL__H

#include <QWidget>
#include<QString>
#include<qvector.h>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include<sstream>
#include<iostream>
#include<QString>
using namespace std;
extern int test;
namespace Ui {
class pl0;
}

class pl0 : public QWidget
{
    Q_OBJECT

public:
    explicit pl0(QWidget *parent = nullptr);
    ~pl0();

    /*--------------符号表---------------*/
    struct table
    {
        int type;
        QString name;
        int level;
        int address;
        int value;
    };
    vector<table>tablelist;
    int tableindex;//符号表中元素个数
    /*--------------四元式---------------*/
    struct quat
    {
        QString opt;
        QString arg1;
        QString arg2;
        QString result;
    };
    vector<quat>quatlist;
    int quatindex;
    QFile tokenfile;
    QFile quatfile;
     /*--------------错误信息---------------*/
    QString errorlist;




     /*--------------语法分析---------------*/
    int curid;//当前读取的token的id,name和line;
    QString curname;
    int curline;
    int curlevel;//现在在第几层
    int curaddress;
    int checksymredef(QString name,int level);//检查某个标识符在本层中是否已经被定义了
    void opsymrefeferr(QString name);//输出重定义信息；
    void checkprog(QFile&tokenfile);//语法分析入口，检查program语句,检查后进入block中
    void checkconst(QFile&tokenfile);//检查const
    void checkoneconst(QFile&tokenfile,bool&errorflag);//检查const中的一个子式
    void opconsterr(bool&errorflag);//输出const的错误信息
    void checkvar(QFile&tokenfile);//检查var
    void checkproc(QFile&tokenfile);//检查procedure
    void checkblock(QFile&tokenfile);//一个模块
    void checkbody(QFile&tokenfile);//检查begin……end
    void checkstatement(QFile&tokenfile);//检查begin……end内的内容；
    void advance(QFile&tokenfile);//读取下一个token；
    void advancelook(QFile&tokenfile);//读取下一个token，获得curid,curname,curline但是文件指针不动
     /*--------------语义分析---------------*/


private:
    Ui::pl0 *ui;
};

#endif // PL__H
