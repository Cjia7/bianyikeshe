#include "pl0.h"
#include "ui_pl0.h"
#include <token.h>
std::vector<pl0::table> pl0::tablelist;

pl0::pl0(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pl0)
{
    ui->setupUi(this);
    tableindex=0;
    curlevel=0;
    quatindex=0;
    curaddress=0;
    tempindex=0;
}
pl0::~pl0()
{
    delete ui;
}


//将begin给出提示

void pl0::advance(QFile&tokenfile)
{
    if(!tokenfile.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Error:Couldn't open the file";
        return;
    }
    else
    {
        if(tokenfile.atEnd())
        {
            qDebug()<<"语法分析结束";
        }
        else
        {
            QString newline;
            newline=tokenfile.readLine();
            int pos1=newline.indexOf(',');
            curname=newline.mid(0,pos1);
            int pos2=newline.indexOf('%');
            curid=(newline.mid(pos1,pos2-pos1)).toInt() ;
            curline=(newline.mid(pos2,newline.length()-pos2)).toInt();
        }
    }
}
void pl0::advancelook(QFile&tokenfile)
{
    if(!tokenfile.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Error:Couldn't open the file";
        return;
    }
    else
    {
        if(tokenfile.atEnd())
        {
            qDebug()<<"语法分析结束";
        }
        else
        {
            int linepos=tokenfile.pos();
            QString newline;
            newline=tokenfile.readLine();
            int pos1=newline.indexOf(',');
            curname=newline.mid(0,pos1);
            int pos2=newline.indexOf('%');
            curid=(newline.mid(pos1,pos2-pos1)).toInt() ;
            curline=(newline.mid(pos2,newline.length()-pos2)).toInt();
            tokenfile.seek(linepos);
        }
    }
}
void pl0::checkarray(QFile&tokenfile){
     advance(tokenfile);//:
     advance(tokenfile);//array
      advance(tokenfile);// [
     advance(tokenfile);//down
      advance(tokenfile);//.
     advance(tokenfile);//.
      advance(tokenfile);//up
      advance(tokenfile);//of
    advance(tokenfile);//int
}

void pl0::checkprog(QFile&tokenfile)
{
    advancelook(tokenfile);
    bool flag=false;//防止错误信息多遍输出
    if(curid==PROGRAM)
    {
        advance(tokenfile);
        advancelook(tokenfile);
        if(curid==SYMBOL)
        {
            QString name=curname;

            advance(tokenfile);
            advancelook(tokenfile);
            if(curid==SEMIC)
            {
                if(checksymredef(name,curlevel)==-1){}
                else
                {
                    errflag=false;
                    opsymrefeferr(curname);
                }
            }
            else
            {
                if(!flag)
                {
                    qDebug()<<"第"<<curline<<"行主过程定义错误;\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行"+"主过程定义错误";
                }
            }

        }
        else
        {
            if(!flag)
            {
                qDebug()<<"第"<<curline<<"行主过程定义错误;\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行"+"主过程定义错误";
            }
        }
    }
    else
    {
        advance(tokenfile);
        qDebug()<<"第"<<curline<<"行，主过程定义错误;\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行"+"主过程定义错误\n";
    }
    flag=false;
    advancelook(tokenfile);
    while(curid!=CON&&curid!=VAR&&curid!=PROCEDURE&&curid!=BEGIN)
    {
        advance(tokenfile);
        advancelook(tokenfile);
    }
    checkblock(tokenfile);
}

bool pl0::checkblock(QFile&tokenfile)
{
    advancelook(tokenfile);
    while(curid!=CON&&curid!=VAR&&curid!=PROCEDURE&&curid!=BEGIN)
    {
        advance(tokenfile);
        advancelook(tokenfile);
        errflag=false;
    }
    if(curid==CON)
    {
        checkconst(tokenfile);
    }
    if(curid==VAR)
    {
        checkvar(tokenfile);
    }
    if(curid==PROCEDURE)
    {
        checkproc(tokenfile);
    }
    if(checkbody(tokenfile)==false)
    {
        errflag=false;
    }
}

//检查标识符是否已经存在于符号表中
int pl0::checksymredef(QString name,int level)
{
    for(int i=0;i<tableindex;i++)
    {
        if(tablelist[i].name==name&&tablelist[i].level==level)return i;
    }
    return -1;
}
//输出标识符重定义的报错信息
void pl0::opsymrefeferr(QString name)
{
    qDebug()<<"第"<<curline<<"行标识符"<<name<<"重定义";
    errorlist=errorlist+"第"+QString::number(curline)+"行标识符重定义\n";
}
void pl0::opconsterr(bool&errorflag)
{
    if(!errorflag)
    {
        qDebug()<<"第"<<curline<<"行，常量定义错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，常量定义错误\n";
        errorflag=true;
    }
    return;
}
bool pl0::checkoneconst(QFile&tokenfile,bool&errorflag)
{
    bool flag=true;
    advancelook(tokenfile);
    if(curid==SYMBOL)
    {
        QString cursymbol=curname;
        advance(tokenfile);
        advancelook(tokenfile);
        if(curid==CEQU)
        {
            advance(tokenfile);
            advancelook(tokenfile);
            if(curid==CONST)
            {
                if(checksymredef(cursymbol,curlevel)==-1){}
                else
                {
                    flag=false;
                    opsymrefeferr(cursymbol);
                }
            }
            else  opconsterr(errorflag);
        }
        else
        {
            flag=false;
            opconsterr(errorflag);
        }
    }
    else
    {
        flag=false;
        opconsterr(errorflag);
    }
    return flag;

}
void pl0::checkconst(QFile&tokenfile)
{
    bool errorflag=false;
    advance(tokenfile);//const
    if(!checkoneconst(tokenfile,errorflag))
        errflag=false;
    advancelook(tokenfile);
    while(curid==COMMA)
    {
        advance(tokenfile);
        if(!checkoneconst(tokenfile,errorflag))
            errflag=false;
    }
    advancelook(tokenfile);
    if(curid==SEMIC){}
    else
    {
        advance(tokenfile);
        if(!errorflag)
        {
            opconsterr(errorflag);
        }
    }
    advancelook(tokenfile);
    while(curid!=VAR&&curid!=PROCEDURE&&curid!=BEGIN)
    {
        errflag=false;
        advance(tokenfile);
        advancelook(tokenfile);
    }
}
void pl0::checkvar(QFile&tokenfile)
{
    advance(tokenfile);
    advancelook(tokenfile);
    bool errorflag=false;//var定义的错误
    if(curid==SYMBOL)
    {
        if(checksymredef(curname,curlevel)==-1){}
        else
        {
            opsymrefeferr(curname);
            errflag=false;
        }
        if(isArrayName(curname.toStdString())){
            checkarray(tokenfile);
        }
        advancelook(tokenfile);
        while(curid==COMMA)
        {
            advance(tokenfile);
            advancelook(tokenfile);
            if(curid==SYMBOL)
            {

                advance(tokenfile);
                if(checksymredef(curname,curlevel)==-1){}
                else
                {
                    opsymrefeferr(curname);
                    errflag=false;
                }
                if(isArrayName(curname.toStdString())){
                    checkarray(tokenfile);
                }
            }
            else
            {
                if(!errorflag)
                {
                    errflag=false;
                    errorflag=true;
                    qDebug()<<"第"<<curline<<"行,变量定义错误\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行,变量定义错误\n";
                }
            }
        }
        if(curid==SEMIC)
        {
            advance(tokenfile);
        }
        else
        {
            if(!errorflag)
            {
                errflag=false;
                errorflag=true;
                qDebug()<<"第"<<curline<<"行变量定义错误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行变量定义错误\n";
            }
        }
    }
    else
    {
        if(!errorflag)
        {
            errflag=false;
            errorflag=true;
            qDebug()<<"第"<<curline<<"行变量定义错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行变量定义错误\n";
        }
    }
    advancelook(tokenfile);
    while(curid!=PROCEDURE&&curid!=BEGIN)
    {
        errflag=false;
        advance(tokenfile);
        advancelook(tokenfile);
    }
}



//符号表：
//int type;     类型
                //类型分为：PROGRAM:主过程，PROCEDURE：过程，CONST:常量，VAR：变量
//QString name;   标识符名称
//int level;     在第几层
//int address;   标记过程的地址，方便call时回填
//int value;    值大小
void pl0::checkproc(QFile&tokenfile)//过程体
{
    bool procflag=true;
    advance(tokenfile);
    advancelook(tokenfile);
    if(curid==SYMBOL)
    {
        QString tempname=curname;
        advance(tokenfile);
        advancelook(tokenfile);
        if(checksymredef(QString::number(curid),curlevel)==-1)
        {
            if(curid==SEMIC){}
            else
            {
                if(procflag)
                {
                    qDebug()<<"第"<<curline<<"行，过程定义错误\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行，过程定义错误;\n";
                    procflag=false;
                }
            }
        }
        else
        {
            qDebug()<<"第"<<curline<<"行，过程名重复\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，过程名重复;\n";
        }
    }
    if(procflag)
    {
        qDebug()<<"第"<<curline<<"行，过程定义错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，过程定义错误;\n";
        procflag=false;
    }
    checkblock(tokenfile);
}

//扫描两遍，check部分检查语法问题，另一个检查语义并生成四元式
bool pl0::checkbody(QFile&tokenfile)//begin……end
{
    bool errorflag=true;
    if(curid==BEGIN)
    {
        advance(tokenfile);
    }
    else
    {
        errorflag=false;
        qDebug()<<"第"<<curline<<"行，过程主体begin错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，过程主体begin错误\n";
    }
    checkstatement(tokenfile);
    advancelook(tokenfile);
    while(curid==SEMIC||(curid!=SEMIC&&curid!=END))
    {
        if(curid==SEMIC)
        {
            advance(tokenfile);
        }
        else
        {
            errorflag=false;
            qDebug()<<"第"<<curline<<"行，缺少;\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少;\n";
        }
        checkstatement(tokenfile);
        advancelook(tokenfile);
    }
    if(curid==END)
    {
        advance(tokenfile);
    }
    else
    {
        errflag=false;
        errorflag=false;
        qDebug()<<"第"<<curline<<"行，过程主体end错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，过程主体end错误\n";
    }
}


int pl0::checkifsymdef()
{
    int symindex=-1;
    if(checksymredef(curname,curlevel)!=-1)symindex=checksymredef(curname,curlevel);
    else if(checksymredef(curname,curlevel-1)!=-1)symindex=checksymredef(curname,curlevel);
    else if(checksymredef(curname,curlevel-2)==-1)symindex=checksymredef(curname,curlevel);
    else
    {
        qDebug()<<"第"<<curline<<"行，标识符未定义\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，标识符未定义\n";
    }
    return symindex;
}
//<statement> → <id> := <exp>
//                       |if <lexp> then <statement>[else <statement>]
//                      |while <lexp> do <statement>
//                       |call <id>（[<exp>{,<exp>}]）
//                       |<body>
//                       |read (<id>{，<id>})
//                       |write (<exp>{,<exp>})
void pl0::checkstatement(QFile&tokenfile)
{
    advancelook(tokenfile);
    while(curid!=SYMBOL&&curid!=IF&&curid!=WHILE&&curid!=BEGIN&&curid!=READ&&curid!=WRITE)
    {
        advance(tokenfile);
        advancelook(tokenfile);
    }
    bool errorflag=true;//没有错误
    if(curid==SYMBOL)//检查赋值表达式
    {
        advance(tokenfile);
        int symindex=-1;
        int temp=checkifsymdef();
        if(temp==-1)errorflag=false;
        else errorflag=true;
        if(symindex!=-1)
        {
            errorflag=false;
            switch(tablelist[symindex].type)
            {
            case VAR:
                break;
            case CONST:
                qDebug()<<"第"<<curline<<"行，赋值语句左侧不可为常量\n";

                break;
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧不可为常量\n";
            case PROGRAM:
                qDebug()<<"第"<<curline<<"行，赋值语句左侧不可为主过程名\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧不可为主过程名\n";
                break;
            case PROCEDURE:
                qDebug()<<"第"<<curline<<"行，赋值语句左侧不可为过程名\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧不可为过程名\n";
                break;
            default:
                qDebug()<<"第"<<curline<<"行，赋值语句左侧应为变量名\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧应为变量名\n";
                break;
            }
        }
        advancelook(tokenfile);
        if(curid!=CEQU)
        {
            errorflag=false;
            qDebug()<<"第"<<curline<<"行，赋值语句中\":=\"错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句中\":=\"错误\n";
        }
        advance(tokenfile);
        int linepos=tokenfile.pos();
        bool temperrflag=checkexpression(tokenfile);
        if(!temperrflag)
        {
            errorflag=false;
            qDebug()<<"第"<<curline<<"行，赋值语句右侧表达式错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句右侧表达式错误\n";
        }
        advancelook(tokenfile);
        if(errorflag&&curid==SEMIC)//如果语法分析没有出现问题，并且后边跟“；”那么进行语义分析
        {
            tokenfile.seek(linepos);
            expression(tokenfile);
            advance(tokenfile);//";"
        }

    }
    if(curid==IF)//检查条件语句
    {
        advance(tokenfile);

    }
    if(curid==WHILE)
    {

    }
    if(curid==BEGIN)
    {

    }
    if(curid==READ)
    {

    }
    if(curid==WRITE)
    {

    }

}
//<statement> →if <lexp> then <statement>[else <statement>]
//<lexp> → <exp> <lop> <exp>|odd <exp>
//<lop> → =|<>|<|<=|>|>=
void pl0::lexp(QFile&tokenfile)
{
   int linepos=tokenfile.pos();
   bool flag=checkexpression(tokenfile);
   if(flag)//没有语法错误
   {
       tokenfile.seek(linepos);
       expression(tokenfile);
   }

}




//表达式的语法分析
//<exp> → [+|-]<term>{<aop><term>} <aop> → +|-
bool pl0::checkexpression(QFile&tokenfile)
{
    advancelook(tokenfile);
    bool flag=true;
    if(curid==ADD||curid==SUB)
    {
        advance(tokenfile);
    }
    flag=checkterm(tokenfile);
    advancelook(tokenfile);
    while((curid==ADD||curid==SUB)&&flag)
    {
        advance(tokenfile);
        flag=checkterm(tokenfile);
        advancelook(tokenfile);
    }
    return flag;
}

//<term> → <factor>{<mop><factor>}
bool pl0:: checkterm(QFile&tokenfile)
{
    bool flag=checkfactor(tokenfile);
    advancelook(tokenfile);
    while((curid==MUL||curid==DIV)&&flag)
    {
        advance(tokenfile);
        flag=checkfactor(tokenfile);
        advancelook(tokenfile);
    }
    return flag;
}

//<factor>→<id>|<integer>|(<exp>)
bool pl0::checkfactor(QFile&tokenfile)
{
    bool flag=true;
    advancelook(tokenfile);
    if(curid==SYMBOL)
    {
        int temp=checkifsymdef();
        if(temp==-1)return false;
        else return true;
    }
    if(curid==CONST)
    {
        return true;
    }
    else if(curid==LBR)
    {
        flag=checkexpression(tokenfile);
        advancelook(tokenfile);
        if(curid==RBR)
        {
            advance(tokenfile);
        }
        else
        {
            qDebug()<<"第"<<curline<<"行，缺少右括号\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少右括号\n";
        }
    }
    return false;
}
//表达式的语义分析
//<exp> → [+|-]<term>{<aop><term>} <aop> → +|-
QString pl0::expression(QFile&tokenfile)
{
    advancelook(tokenfile);

    bool isminus=false;
    if(curid==ADD)
    {
        advance(tokenfile);
    }
    else if(curid==SUB)
    {
        advance(tokenfile);
        isminus=true;
    }
    QString explace=term(tokenfile);
    if(isminus)
    {
        QString tempstring=newtemp();
        quatemit("MINUS",explace,"-",tempstring);
        explace=tempstring;
    }
    advancelook(tokenfile);
    while(curid==ADD||curid==SUB)
    {
        QString name=curname;
        advance(tokenfile);
        QString tempstring=newtemp();
        QString termstr=term(tokenfile);
        quatemit(name,explace,termstr,tempstring);
        explace=tempstring;
        advancelook(tokenfile);
    }
    return explace;
}

//<term> → <factor>{<mop><factor>}
QString pl0::term(QFile&tokenfile)
{
    QString explace=factor(tokenfile);
    advancelook(tokenfile);
    while(curid==MUL||curid==DIV)
    {
        advance(tokenfile);
        QString mark=curname;
        QString facstring=factor(tokenfile);
        QString tempstring=newtemp();
        quatemit(curname,explace,facstring,tempstring);
        explace=tempstring;
        advancelook(tokenfile);
    }
    return explace;
}

//<factor>→<id>|<integer>|(<exp>)
QString pl0::factor(QFile&tokenfile)
{
    advancelook(tokenfile);
    if(curid==SYMBOL||curid==CONST)
    {
        return curname;
    }
    else if(curid==LBR)
    {
        advance(tokenfile);
        return expression(tokenfile);
    }
}

QString pl0::newtemp()
{
    tempindex++;
    return "t"+QString::number(tempindex);
}
void pl0::quatemit(QString opt,QString arg1,QString arg2,QString result)
{
    quatlist[quatindex].opt=opt;
    quatlist[quatindex].arg1=arg1;
    quatlist[quatindex].arg2=arg2;
    quatlist[quatindex++].result=result;
}


//<lexp> → <exp> <lop> <exp>|odd <exp>












