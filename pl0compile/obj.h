#ifndef OBJ_H
#define OBJ_H
#include <QString>
#include <vector>
#include <map>
#include <utility>
#include "dag.h"

//一条汇编指令
class Instruction
{
public:
    Instruction(QString op, QString name1, QString name2): op(op), name1(name1), name2(name2), label()  {}
    Instruction(QString op, QString name1, QString name2, QString label): op(op), name1(name1), name2(name2), label(label)  {}
    /* Instruction(std::string op, std::string name1, std::string name2): op(op), name1(name1), name2(name2), label()  {}
    Instruction(std::string op, std::string name1, std::string name2, std::string label): op(op), name1(name1), name2(name2), label(label)  {} */

    QString label;
    QString op;
    QString name1;
    QString name2;  
};

//用来指示变量存放位置的结构体
//pos 表示具体的位置（如寄存器名或内存地址），isMem 是一个布尔值，指示该位置是否在内存中
class Position
{
public:
    Position() = default;
    Position(QString pos, bool isMem): pos(pos), isMem(isMem) {};
    QString pos;
    bool isMem;
};

class OBJECT
{
public:
    struct Record{
        int begin;
        int end;
    };
    struct SymbolInfo {
        int pos; //符号位置
        bool isIN;   // 是否在符号表
    };
    OBJECT()= default;
    ~OBJECT()= default;
    void geneASM();
private:
    std::vector<OPTIMIZE::BasicBlock> Rblocks;
    //没有AL是因为为了方便，AX寄存器保留作为乘除法运算单元与返回值保存寄存器
    //RDL （寄存器描述列表）
    std::map<QString, QString> RDL = { std::make_pair("BL", ""),
                               std::make_pair("CL", ""),
                               std::make_pair("DL", "") };
    //posMap （变量位置表）
    std::map<QString, Position> posMap;
    //instVec （指令）
    std::vector<Instruction> instVec;
private:
    bool isActive(const QString& name,QString curFun);
    bool isOperator(const OPTIMIZE::OperatorType& qt);
    bool isNum(const QString& str);
    QString getAddr(const QString &name, const QString &curFun);
    QString findEmpty();
    void saveR(const QString& curFun);
    void getR(const OPTIMIZE::QuadTuple& qt, QString& R, QString& B, QString& C, const QString& curFun);
    SymbolInfo getSymbolInfo(const QString& name, const QString& curFun);
};

#endif // OBJ_H

