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
QT_END_NAMESPACE
class pl0 : public QWidget
{
    Q_OBJECT

public:
    explicit pl0(QWidget *parent = nullptr);
    ~pl0();
    /*--------------与词法分析的链接---------------*/

    /*--------------符号表---------------*/
    struct table
    {
        int type;
        QString name;
        int level;
        int address;
        int value;
    };
    static vector<table>tablelist;
    int tableindex;//符号表中元素个数
    int curaddress;//标识符的地址
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
    bool errflag;//初始为true,表示无错误；若出现错误则为false,不进行语义分析



     /*--------------语法分析---------------*/
    int curid;//当前读取的token的id,name和line;
    QString curname;
    int curline;
    int curlevel;//现在在第几层
    int tempindex;//临时变量当前数目
    int checksymredef(QString name,int level);//检查某个标识符在本层中是否已经被定义了
    void opsymrefeferr(QString name);//输出重定义信息；
    void checkprog(QFile&tokenfile);//语法分析入口，检查program语句,检查后进入block中
    bool checkblock(QFile&tokenfile);//一个模块
    void checkconst(QFile&tokenfile);//检查const
    bool checkoneconst(QFile&tokenfile,bool&errorflag);//检查const中的一个子式
    void opconsterr(bool&errorflag);//输出const的错误信息
    void checkvar(QFile&tokenfile);//检查var
    void checkproc(QFile&tokenfile);//检查procedure
    bool checkbody(QFile&tokenfile);//检查begin……end
    void checkstatement(QFile&tokenfile);//检查begin……end内的内容；
    bool checkexpression(QFile&tokenfile);
    bool checkterm(QFile&tokenfile);
    bool checkfactor(QFile&tokenfile);//算数表达式的语法分析
    void checklexp(QFile&tokenfile);//检查判断条件
    int checkifsymdef();//检查标识符是否被定义过
    void advance(QFile&tokenfile);//读取下一个token；
    void advancelook(QFile&tokenfile);//读取下一个token，获得curid,curname,curline但是文件指针不动


    /*--------------语义分析---------------*/
    void prog(QFile&tokenfile);
    void block(QFile&tokenfile);
    void condecl(QFile&tokenfile);
    void vardecl(QFile&tokenfile);
    void proc(QFile&tokenfile);
    void body(QFile&tokenfile);
    void statement(QFile&tokenfile);
    QString expression(QFile&tokenfile);//<exp> → [+|-]<term>{<aop><term>}
    QString term(QFile&tokenfile);//<term> → <factor>{<mop><factor>}
    QString factor(QFile&tokenfile);//<factor>→<id>|<integer>|(<exp>)
    QString newtemp();//返回新的临时变量
    QString lexp(QFile&tokenfile);//判断条件
    void lop(QFile&tokenfile);
    void quatemit(QString opt,QString arg1,QString arg2,QString result);//填入四元式

    /*--------------与目标代码生成的链接---------------*/
    void writeQuatListToFile(const std::vector<quat>& quatlist);

private:
    Ui::pl0 *ui;
};

#endif // PL__H
