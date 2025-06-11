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
    : QWidget(parent)  // 这里 parent 是传入的 QWidget*
    , ui(new Ui::pl0)
{
    ui->setupUi(this);  // 这里 this 是 pl0*，可以正常使用
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
                if(checksymredef(name,curlevel)==-1)
                {
                    tablelist[tableindex].name=name;
                    tablelist[tableindex].level=curlevel;
                    tablelist[tableindex++].type=PROGRAM;
                    //   tablelist[tableindex++].address=curaddress++;
                }
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
void pl0::prog(QFile&tokenfile)
{
    advance(tokenfile);//program
    advance(tokenfile);//symbol
    quatlist[quatindex].opt="PROG";
    quatlist[quatindex].arg1=curname;
    advance(tokenfile);//;
    block(tokenfile);
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
        curlevel++;
        checkproc(tokenfile);
        curlevel--;
    }
    if(checkbody(tokenfile)==false)
    {
        errflag=false;
    }
}
void pl0::block(QFile&tokenfile)
{
    advance(tokenfile);
    if(curid==CON)
    {
        condecl(tokenfile);
    }
    if(curid==VAR)
    {
        vardecl(tokenfile);
    }
    if(curid==PROCEDURE)
    {
        curlevel++;
        proc(tokenfile);
        curlevel--;
    }
    body(tokenfile);
}
//<condecl> → const <const>{,<const>};
//<const> → <id>:=<integer>
void pl0::condecl(QFile&tokenfile)
{
    advance(tokenfile);//第一个常数标识符
    QString name=curname;
    advance(tokenfile);//:=
    advance(tokenfile);//常量值
    quatlist[quatindex].opt=":=";
    quatlist[quatindex].arg1=curname;
    quatlist[quatindex++].result=name;
    advance(tokenfile);//
    while(curid==COMMA)
    {
        advance(tokenfile);//常数标识符
        name=curname;
        advance(tokenfile);//:=
        advance(tokenfile);//常量值
        quatlist[quatindex].opt=":=";
        quatlist[quatindex].arg1=curname;
        quatlist[quatindex++].result=name;
        advance(tokenfile);//
    }
}
//<vardecl> → var <id>{,<id>};
void pl0::vardecl(QFile&tokenfile)
{
    advance(tokenfile);
    quatlist[quatindex].opt="vardef";
    quatlist[quatindex++].arg1=curname;
    advance(tokenfile);
    while(curid==COMMA)
    {
        advance(tokenfile);
        quatlist[quatindex].opt="vardef";
        quatlist[quatindex++].arg1=curname;
        advance(tokenfile);
    }
}
//<proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
void pl0::proc(QFile&tokenfile)
{
    advance(tokenfile);
    quatlist[quatindex].opt="prodef";
    quatlist[quatindex].arg1=curname;

    advance(tokenfile);
    block(tokenfile);
    advancelook(tokenfile);
    while(curid==SEMIC)
    {
        advance(tokenfile);
        proc(tokenfile);
        advancelook(tokenfile);
    }
}
//<body> → begin <statement>{;<statement>}end
void pl0::body(QFile&tokenfile)
{
    advance(tokenfile);
    QString name="progbegin";
    if(curlevel!=0)//表示不是主过程
    {
        name="procbegin";
    }
    quatlist[quatindex].opt=name;
    statement(tokenfile);
    advancelook(tokenfile);
    while(curid==SEMIC)
    {
        advance(tokenfile);
        statement(tokenfile);
        advancelook(tokenfile);
    }
    advance(tokenfile);
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
                if(checksymredef(cursymbol,curlevel)==-1)
                {
                    tablelist[tableindex].name=cursymbol;
                    tablelist[tableindex].level=curlevel;
                    tablelist[tableindex].type=CONST;
                    tablelist[tableindex++].value=curname.toInt();
                }
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
        if(checksymredef(curname,curlevel)==-1)
        {
            tablelist[tableindex].name=curname;
            tablelist[tableindex].level=curlevel;
            tablelist[tableindex].type=VAR;
            tablelist[tableindex++].address=curaddress++;
        }
        else
        {
            opsymrefeferr(curname);
            errflag=false;
        }
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
                    tablelist[tableindex].type=VAR;
                    tablelist[tableindex++].address=curaddress++;
                }
                else
                {
                    opsymrefeferr(curname);
                    errflag=false;
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
    // advancelook(tokenfile);
    // while(curid!=PROCEDURE&&curid!=BEGIN)
    // {
    //     errflag=false;
    //     advance(tokenfile);
    //     advancelook(tokenfile);
    // }
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
            if(curid==SEMIC)
            {
                tablelist[tableindex].name=curname;
                tablelist[tableindex].level=curlevel;
                tablelist[tableindex++].type=PROCEDURE;
                //  tablelist[tableindex++].address=curaddress++;
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
    checkblock(tokenfile);
    advancelook(tokenfile);
    //<proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
    while(curid==SEMIC)
    {
        advance(tokenfile);
        checkproc(tokenfile);
    }

}

//<body> → begin <statement>{;<statement>}end
bool pl0::checkbody(QFile&tokenfile)//begin……end
{
    bool errorflag=true;
    if(curid==BEGIN)
    {
        advance(tokenfile);
    }
    else
    {
        errflag=false;
        errorflag=false;
        qDebug()<<"第"<<curline<<"行，过程主体begin错误\n";
        errorlist=errorlist+"第"+QString::number(curline)+"行，过程主体begin错误\n";
    }
    checkstatement(tokenfile);
    advancelook(tokenfile);
    while(curid==SEMIC)
    {
        advance(tokenfile);
        checkstatement(tokenfile);
    }
    advancelook(tokenfile);
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
//                       |while <lexp> do <statement>
//                       |call <id>（[<exp>{,<exp>}]）
//                       |<body>
//                       |read (<id>{，<id>})
//                       |write (<exp>{,<exp>})
void pl0::statement(QFile&tokenfile)
{
    advance(tokenfile);
    if(curid==SYMBOL)
    {
        QString name=curname;
        advance(tokenfile);
        expression(tokenfile);
    }
    else if(curid==IF)
    {
        QString arg1=lexp(tokenfile);
        quatemit("IF",arg1,"_","_");
        advance(tokenfile);//then
        statement(tokenfile);
        quatemit("IFEND","_","_","_");
        advancelook(tokenfile);
        if(curid==ELSE)
        {
            advance(tokenfile);
            statement(tokenfile);
        }
    }
    else if(curid==WHILE)
    {
        quatemit("WHILE","_","_","_");
        QString arg1=lexp(tokenfile);
        advance(tokenfile);
        quatemit("DO",arg1,"_","_");
        statement(tokenfile);
    }
    else if(curid==CALL)
    {
        advance(tokenfile);
        quatlist[quatindex].opt="CALL";
        quatlist[quatindex++].arg1=curname;
    }
    else if(curid==READ)
    {
        advance(tokenfile);//左括弧
        advance(tokenfile);//第一个id
        quatlist[quatindex].opt="READ";
        quatlist[quatindex++].result=curname;
        advance(tokenfile);//,或是)
        while(curid==SEMIC)
        {
            advance(tokenfile);//读到下一个id
            quatlist[quatindex].opt="READ";
            quatlist[quatindex++].result=curname;
            advance(tokenfile);
        }
    }
    else if(curid==WRITE)
    {//|write (<exp>{,<exp>})
        advance(tokenfile);//左括弧
        QString writestr=expression(tokenfile);
        quatlist[quatindex].opt="WRITE";;
        quatlist[quatindex++].result=writestr;
        advance(tokenfile);
        while(curid==SEMIC)
        {
            writestr=expression(tokenfile);
            quatlist[quatindex].opt="write";
            quatlist[quatindex++].result=writestr;
            advance(tokenfile);
        }
    }
    else
    {
        body(tokenfile);
    }
}
void pl0::checkstatement(QFile&tokenfile)
{
    advancelook(tokenfile);
    while(curid!=SYMBOL&&curid!=IF&&curid!=WHILE&&curid!=CALL&&curid!=BEGIN&&curid!=READ&&curid!=WRITE)
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
                errflag=false;
                qDebug()<<"第"<<curline<<"行，赋值语句左侧不可为常量\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧不可为常量\n";
                break;
            case PROGRAM:
                errflag=false;
                qDebug()<<"第"<<curline<<"行，赋值语句左侧不可为主过程名\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧不可为主过程名\n";
                break;
            case PROCEDURE:
                errflag=false;
                qDebug()<<"第"<<curline<<"行，赋值语句左侧不可为过程名\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧不可为过程名\n";
                break;
            default:
                errflag=false;
                qDebug()<<"第"<<curline<<"行，赋值语句左侧应为变量名\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句左侧应为变量名\n";
                break;
            }
        }
        advancelook(tokenfile);
        if(curid!=CEQU)
        {
            errflag=false;
            errorflag=false;
            qDebug()<<"第"<<curline<<"行，赋值语句中\":=\"错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句中\":=\"错误\n";
        }
        else
        {
            advance(tokenfile);
        }
        advancelook(tokenfile);
        while(curid!=SYMBOL&&curid!=CONST&&curid!=LBR&&curid!=SUB&&curid!=ADD)
        {
            errflag=false;
            advance(tokenfile);
            advancelook(tokenfile);
        }
        if(!checkexpression(tokenfile))
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，赋值语句右侧错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，赋值语句右侧错误\n";
        }
    }
    //<statement> →if <lexp> then <statement>[else <statement>]
    if(curid==IF)//检查条件语句
    {
        advance(tokenfile);
        checklexp(tokenfile);
        advancelook(tokenfile);
        if(curid==THEN)
        {
            advance(tokenfile);
        }
        else
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，缺少then\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少then\n";
        }
        checkstatement(tokenfile);
        advancelook(tokenfile);
        if(curid==ELSE)
        {
            advance(tokenfile);
            checkstatement(tokenfile);
        }
        else if(curid==SEMIC)
        {
            advance(tokenfile);
        }
        else
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，条件语句中存在错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，条件语句中存在错误\n";
        }

    }
    //while <lexp> do <statement>
    if(curid==WHILE)
    {
        advance(tokenfile);
        checklexp(tokenfile);
        advancelook(tokenfile);
        if(curid==DO)
        {
            advance(tokenfile);
            checkstatement(tokenfile);
            advancelook(tokenfile);
            if(curid==SEMIC){}
            else
            {
                errflag=false;
                qDebug()<<"第"<<curline<<"行，循环语句中存在错误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，循环语句中存在错误\n";
            }
        }
        else
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，循环语句中存在错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，循环语句中存在错误\n";
        }
    }
    if(curid==CALL)
    {
        advance(tokenfile);
        advancelook(tokenfile);
        if(curid==SYMBOL)
        {
            advance(tokenfile);
            if(checkifsymdef()==-1)//未被定义的变量
            {
                errflag=false;
                qDebug()<<"第"<<curline<<"行，READ语句中标识符未被定义\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，READ语句中标识符未被定义\n";
            }
            else
            {
                int index=checkifsymdef();
                if(tablelist[index].type!=PROCEDURE)
                {
                    errflag=false;
                    qDebug()<<"第"<<curline<<"行，CALL语句中标识符类型应为过程名\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行，CALL语句中标识符类型应为过程名\n";
                }
            }
        }
        else
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，CALL语句语义错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，CALL语句语义错误\n";
        }
    }
    //<statement> →<body>
    if(curid==BEGIN)
    {
        checkbody(tokenfile);
    }
    //read (<id>{，<id>})
    if(curid==READ)
    {
        //  bool readflag=true;
        advance(tokenfile);
        advancelook(tokenfile);
        if(curid==SYMBOL)
        {
            advance(tokenfile);
            if(checkifsymdef()==-1)//未被定义的变量
            {
                errflag=false;
                qDebug()<<"第"<<curline<<"行，READ语句中标识符未被定义\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，READ语句中标识符未被定义\n";
            }
            else
            {
                int index=checkifsymdef();
                if(tablelist[index].type!=VAR)
                {
                    errflag=false;
                    qDebug()<<"第"<<curline<<"行，READ语句中标识符类型应为变量\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行，READ语句中标识符类型应为变量\n";
                }
            }
        }
        else
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，READ语句中存在错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，READ语句中存在错误\n";
        }
        advancelook(tokenfile);
        while(curid==COMMA)
        {
            advance(tokenfile);
            advancelook(tokenfile);
            if(curid==SYMBOL)
            {
                advance(tokenfile);
                if(checkifsymdef()==-1)//未被定义的变量
                {
                    errflag=false;
                    qDebug()<<"第"<<curline<<"行，READ语句中标识符未被定义\n";
                    errorlist=errorlist+"第"+QString::number(curline)+"行，READ语句中标识符未被定义\n";
                }
                else
                {
                    int index=checkifsymdef();
                    if(tablelist[index].type!=VAR)
                    {
                        errflag=false;
                        qDebug()<<"第"<<curline<<"行，READ语句中标识符类型应为变量\n";
                        errorlist=errorlist+"第"+QString::number(curline)+"行，READ语句中标识符类型应为变量\n";
                    }
                }
            }
        }
        advancelook(tokenfile);
        if(curid!=SEMIC)
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，缺少\";\"\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少\";\"\n";
        }
    }
    //write (<exp>{,<exp>})
    if(curid==WRITE)
    {
        advance(tokenfile);
        advancelook(tokenfile);
        if(curid==LBR)
        {
            advance(tokenfile);
        }
        else
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，缺少左括弧\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少左括弧\n";
        }
        while(curid!=SYMBOL&&curid!=CONST&&curid!=LBR&&curid!=SUB&&curid!=ADD)//删除无用符，进入
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，语义错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，语义错误\n";
            advance(tokenfile);
            advancelook(tokenfile);
        }
        if(checkexpression(tokenfile)==false)
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，表达式语义错误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，表达式语义错误\n";
        }
        advancelook(tokenfile);
        while(curid==COMMA)
        {
            advance(tokenfile);
            while(curid!=SYMBOL&&curid!=CONST&&curid!=LBR&&curid!=SUB&&curid!=ADD)//删除无用符，进入
            {
                errflag=false;
                qDebug()<<"第"<<curline<<"行，语义错误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，语义错误\n";
                advance(tokenfile);
                advancelook(tokenfile);
            }
            if(checkexpression(tokenfile)==false)
            {
                errflag=false;
                qDebug()<<"第"<<curline<<"行，表达式语义错误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，表达式语义错误\n";
            }
            advancelook(tokenfile);
        }
        advancelook(tokenfile);
        if(curid!=RBR)
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，缺少\")\"\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少\")\"\n";
        }
        else advance(tokenfile);
        if(curid!=SEMIC)
        {
            errflag=false;
            qDebug()<<"第"<<curline<<"行，缺少\";\"\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，缺少\";\"\n";
        }
        else advance(tokenfile);
    }

}
//<lexp> → <exp> <lop> <exp>|odd <exp>
//<lop> → =|<>|<|<=|>|>=
void pl0::checklexp(QFile&tokenfile)
{
    bool errorflag=true;
    while(curid!=SYMBOL&&curid!=CONST&&curid!=LBR&&curid!=SUB&&curid!=ADD&&curid!=ODD)//删除无用符，进入
    {
        if(errorflag)
        {
            qDebug()<<"第"<<curline<<"行，判断条件有误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，判断条件有误\n";
            errorflag=false;
            advancelook(tokenfile);
            while(curid!=THEN&&curid!=DO)
            {
                advance(tokenfile);
                advancelook(tokenfile);
            }
            return ;
        }
        errflag=false;
        advance(tokenfile);
        advancelook(tokenfile);
    }
    advancelook(tokenfile);
    if(curid==ODD)
    {
        while(curid!=SYMBOL&&curid!=CONST&&curid!=LBR&&curid!=SUB&&curid!=ADD)//无法进入expression
        {
            if(errorflag)
            {
                qDebug()<<"第"<<curline<<"行，判断条件有误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，判断条件有误\n";
                errorflag=false;
                advancelook(tokenfile);
                while(curid!=THEN&&curid!=DO)
                {
                    advance(tokenfile);
                    advancelook(tokenfile);
                }
                return ;
            }
            errflag=false;
            advance(tokenfile);
            advancelook(tokenfile);
        }
        if(checkexpression(tokenfile))
        {
            return;
        }
        else
        {
            if(errorflag)
            {
                qDebug()<<"第"<<curline<<"行，判断条件有误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，判断条件有误\n";
                errorflag=false;
                advancelook(tokenfile);
                while(curid!=THEN&&curid!=DO)
                {
                    advance(tokenfile);
                    advancelook(tokenfile);
                }
                return ;
            }
        }
    }
    else//expression
    {
        if(!checkexpression(tokenfile)&&errorflag)//判断条件有误
        {
            qDebug()<<"第"<<curline<<"行，判断条件有误\n";
            errorlist=errorlist+"第"+QString::number(curline)+"行，判断条件有误\n";
            errorflag=false;
            while(curid!=THEN&&curid!=DO)
            {
                advance(tokenfile);
                advancelook(tokenfile);
            }
            return ;
        }
        advancelook(tokenfile);
        if(curid==EQU||curid==LES||curid==LESE||curid==LARE||curid==LAR||curid==NEQU)
        {
            advance(tokenfile);
            if(!checkexpression(tokenfile)&&errorflag)//判断条件有误
            {
                qDebug()<<"第"<<curline<<"行，判断条件有误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，判断条件有误\n";
                errorflag=false;
                while(curid!=THEN&&curid!=DO)
                {
                    advance(tokenfile);
                    advancelook(tokenfile);
                }
                return ;
            }
        }
        else
        {
            if(errorflag)
            {
                qDebug()<<"第"<<curline<<"行，判断条件有误\n";
                errorlist=errorlist+"第"+QString::number(curline)+"行，判断条件有误\n";
                errorflag=false;
            }
            while(curid!=THEN&&curid!=DO)
            {
                advance(tokenfile);
                advancelook(tokenfile);
            }
            return ;
        }
    }
}
//<statement> →if <lexp> then <statement>[else <statement>]
//<lexp> → <exp> <lop> <exp>|odd <exp>
//<lop> → =|<>|<|<=|>|>=
QString pl0::lexp(QFile&tokenfile)
{
    advancelook(tokenfile);
    if(curid==ODD)
    {
        advance(tokenfile);
        QString arg1=expression(tokenfile);
        QString result=newtemp();
        quatemit("MOD",arg1,"2",result);
    }
    else
    {
        QString arg1=expression(tokenfile);
        advance(tokenfile);
        QString opt=curname;
        QString arg2=expression(tokenfile);
        QString result=newtemp();
        quatemit(opt,arg1,arg2,result);
        return result;
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
        if(temp==-1)return false;//检测变量是否是已经被定义的
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
            return true;
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
        quatemit("MINUS",explace,"_",tempstring);
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
        advance(tokenfile);
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












