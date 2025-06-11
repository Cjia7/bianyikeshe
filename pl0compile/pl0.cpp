#include "pl0.h"
#include "ui_pl0.h"

std::vector<pl0::table> pl0::tablelist;
typedef enum {
    PROGRAM = 1,      // "program"
    BEGIN,            // "begin"
    END,              // "end"
    IF,               // "if"
    THEN,             // "then"
    ELSE,             // "else"
    CON,              // "const"
    PROCEDURE,        // "procedure"
    VAR,              // "var"
    DO,               // "do"
    WHILE,            // "while"
    CALL,             // "call"
    READ,             // "read"
    WRITE,            // "write"
    REPEAT,           // "repeat"
    ODD,              // "odd"

    EQU,              // "="
    LES,              // "<"
    LESE,             // "<="
    LARE,             // ">="
    LAR,              // ">"
    NEQU,             // "<>"
    ADD,              // "+"
    SUB,              // "-"
    MUL,              // "*"
    DIV,              // "/"

    SYMBOL,           // 标识符
    CONST,            // 常量
    CEQU,             // ":="
    COMMA,            // ","
    SEMIC,            // ";"
    POI,              // "."
    LBR,              // "("
    RBR               // ")"
}TokenType;
extern TokenType tokentype;
pl0::pl0(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pl0)
{
    ui->setupUi(this);
    tableindex=0;
    curlevel=0;
    quatindex=0;
    curaddress=0;
}
pl0::~pl0()
{
    delete ui;
}
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

void pl0::checkprog(QFile&tokenfile)
{
    advancelook(tokenfile);
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
                if(checksymredef(name,curlevel)==-1)
                {
                    advance(tokenfile);
                    tablelist[tableindex].name==name;
                    tablelist[tableindex].type=curid;
                    tablelist[tableindex].level=curlevel;
                    tableindex++;
                    quatlist[quatindex].opt="prog";
                    quatlist[quatindex].arg1=curname;
                }
               else
                {
                    opsymrefeferr(curname);
                }
            }
            else
            {
                if(curid==CON||curid==PROCEDURE||curid==VAR||curid==BEGIN)
                {
                    qDebug()<<"第"<<curline<<"行缺少;\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行"+"缺少;\n";
                }
                else
                {
                    qDebug()<<"第"<<curline<<"行出现错误\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行"+"出现错误\n";

                }
            }
        }
        else if(curid==SEMIC)
        {
            advance(tokenfile);
            qDebug()<<"第"<<curline<<"行标识符定义错误;\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行"+"标识符定义错误\n";
        }
        else
        {
            qDebug()<<"第"<<curline<<"行标识符定义错误;\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行"+"标识符定义错误";
        }
    }
    else if(curid==SYMBOL)
    {
        advance(tokenfile);
        qDebug()<<"第"<<QString::number(curline)<<"行缺少program\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行"+"缺少program\n";
        advancelook(tokenfile);
        if(curid==SEMIC)
        {
            advance(tokenfile);
        }
        else
        {
            qDebug()<<"第"<<QString::number(curline-1)<<"行缺少;\n";
            errorlist=errorlist+"第"+QString::number(curline-1)+"行"+"缺少;\n";
        }
    }
    else if(curid==CON||curid==PROCEDURE||curid==VAR||curid==BEGIN)
    {

        qDebug()<<"第"<<QString::number(curline)<<"行缺少program\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行"+"缺少program\n";
    }
    else
    {
        qDebug()<<"第"<<QString::number(curline)<<"行错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行错误\n";
    }

    checkblock(tokenfile);
}

void pl0::checkblock(QFile&tokenfile)
{
    advancelook(tokenfile);
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
    checkbody(tokenfile);
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
        qDebug()<<"第"<<curline<<"行，常数定义错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，常数定义错误\n";
        errorflag=true;
    }
    return;
}
void pl0::checkoneconst(QFile&tokenfile,bool&errorflag)
{
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
                if(checksymredef(cursymbol,curlevel)==-1)
                {
                    advance(tokenfile);
                    tablelist[tableindex].name=cursymbol;
                    tablelist[tableindex].type=CONST;
                    tablelist[tableindex].level=curlevel;
                    tablelist[tableindex++].value=curname.toInt();
                    quatlist[quatindex].opt=":=";
                    quatlist[quatindex].arg1=cursymbol;
                    quatlist[quatindex++].result=curname;
                }
                else
                {
                    opsymrefeferr(cursymbol);
                }

            }
            else
            {
                opconsterr(errorflag);
            }

        }
        else
        {
            opconsterr(errorflag);
        }

    }
    else
    {
        opconsterr(errorflag);
    }


}
void pl0::checkconst(QFile&tokenfile)
{
    bool errorflag=false;
    advance(tokenfile);
    checkoneconst(tokenfile,errorflag);
    advancelook(tokenfile);
    while(curid==COMMA)
    {
        advance(tokenfile);
        checkoneconst(tokenfile,errorflag);
    }
    advancelook(tokenfile);
    if(curid==SEMIC)
    {
        return ;
    }
    else
    {
        if(!errorflag)
        {
            opconsterr(errorflag);
        }

        return;
    }
}


void pl0::checkvar(QFile&tokenfile)
{
    advance(tokenfile);
    advancelook(tokenfile);
    bool errorflag=false;//var定义的错误
    if(curid==SYMBOL)
    {
        if(checksymredef(curname,curlevel)==-1)
        {
            tablelist[tableindex].name=curname;
            tablelist[tableindex].level=curlevel;
            tablelist[tableindex++].type=curid;
            quatlist[quatindex].opt="vardef";
            quatlist[quatindex++].arg1=curname;
        }
        else opsymrefeferr(curname);
        advancelook(tokenfile);
        while(curid==COMMA)
        {
            advance(tokenfile);
            advancelook(tokenfile);
            if(curid==SYMBOL)
            {
                advance(tokenfile);
                if(checksymredef(curname,curlevel)==-1)
                {
                    tablelist[tableindex].name=curname;
                    tablelist[tableindex].level=curlevel;
                    tablelist[tableindex++].type=curid;
                    quatlist[quatindex].opt="vardef";
                    quatlist[quatindex++].arg1=curname;
                }
                else opsymrefeferr(curname);
            }
            else
            {
                if(!errorflag)
                {
                    errorflag=true;
                    qDebug()<<"第"<<curline<<"行变量定义错误\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行变量定义错误\n";
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
            errorflag=true;
            qDebug()<<"第"<<curline<<"行变量定义错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行变量定义错误\n";
        }
    }
}



//符号表：
//int type;     类型
                //类型分为：PROGRAM:主过程，PROCEDURE：过程，CONST:常量，VAR：变量
//QString name;   标识符名称
//int level;     在第几层
//int address;   标记过程的地址，方便call时回填
//int value;    值大小
void pl0::checkproc(QFile&tokenfile)
{
    bool procflag=true;
    advance(tokenfile);
    advancelook(tokenfile);
    if(curid==SYMBOL)
    {
        advance(tokenfile);
        advancelook(tokenfile);
        if(checksymredef(QString::number(curid),curlevel)==-1)
        {
            if(curid==SEMIC)
            {
                curlevel++;
                tablelist[tableindex].type=PROCEDURE;
                tablelist[tableindex].name=curname;
                tablelist[tableindex].level=curlevel;
                checkblock(tokenfile);
                curlevel--;
            }
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
}

void pl0::checkbody(QFile&tokenfile)
{
    if(curid==BEGIN)
    {
        advance(tokenfile);
        checkstatement(tokenfile);
        advancelook(tokenfile);
        if(curid==END)
        {
            advance(tokenfile);
            return;
        }
        else
        {
            qDebug()<<"第"<<curline<<"行，过程主体end错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，过程主体end错误\n";
        }
    }
    else
    {
        qDebug()<<"第"<<curline<<"行，过程主体begin错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，过程主体begin错误\n";
    }
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

}














