#include <map>
#include <vector>
#include <stack>
#include <iostream>
#include <QMessageBox>
#include "pl0.h"
#include "obj.h"
#include "dag.h"
using namespace std;

void OBJECT::geneASM()
{
    //先写八股
    instVec.emplace_back("SEGMENT", "", "", "DSEG");
    instVec.emplace_back("DB", "0", "", "TEMP");//用来存临时变量
    for (const auto& array :arrayNames) {
        int size = array.up - array.down + 1;
        instVec.emplace_back("DB", QString::number(size) + " DUP(0)", "", array.name);
    }
    instVec.emplace_back("DB", "0", "", "DATA");//数据
    instVec.emplace_back("ENDS", "", "", "DSEG");
    instVec.emplace_back("SEGMENT", "", "", "SSEG");
    instVec.emplace_back("DB", "100   DUP(0)", "", "AR");//活动记录所在的栈区域
    instVec.emplace_back("ENDS", "", "", "SSEG");
    instVec.emplace_back("SEGMENT", "", "", "CSEG");
    instVec.emplace_back("ASSUME", "CS:CSEG", "", "");
    instVec.emplace_back("ASSUME", "DS:DSEG", "", "");
    instVec.emplace_back("ASSUME", "SS:SSEG", "", "");

    stack<int> sem;//用来记录返填信息的
    vector<Record> record;
    Record rec;
    int labelNow = 0; //用来记录当前label标号

    //遍历所有基本块
    for (auto block : Rblocks) {
        OPTIMIZE::OperatorType cmpType;//用于编写if和do的跳转指令
        saveR(block.curFun);
        posMap.clear();
        //一个基本块开始前，先基本块内所有的变量都设为内存变量
        for (auto qt : block.basic_block) {
            if (isOperator(qt.op)) {
                posMap[qt.arg1] = Position(qt.arg1, true);
                if (qt.arg2 != "__") posMap[qt.arg2] = Position(qt.arg2, true);
                posMap[qt.result] = Position(qt.result, true);
            }
        }
        for (auto qt : block.basic_block) {
            if (isOperator(qt.op)) {
                QString R, B, C;
                getR(qt, R, B, C, block.curFun);
                if (qt.op == OPTIMIZE::OperatorType::ASSIGN) {
                    //如果是赋值语句
                    if (R != B) instVec.emplace_back("MOV", R, B);
                    posMap[qt.result] = Position(R, false);
                    RDL[R] = qt.result;
                }
                else {
                    if (R != B) instVec.emplace_back("MOV", R, B);
                    if (qt.op == OPTIMIZE::OperatorType::MUL) {
                        instVec.emplace_back("MOV", "AL", B);
                        instVec.emplace_back("MOV", "AH", "0");
                        if (isNum(C)) {
                            //MUL不能接立即数，因此需要将此立即数存起来
                            instVec.emplace_back("MOV", "TEMP", C);
                            instVec.emplace_back("MUL", "TEMP", "");
                        }
                        else {
                            instVec.emplace_back("MUL", C, "");
                        }
                        instVec.emplace_back("MOV", R, "AL");
                    }
                    else if (qt.op == OPTIMIZE::OperatorType::DIV) {
                        instVec.emplace_back("MOV", "AL", B);
                        instVec.emplace_back("MOV", "AH", "0");
                        if (isNum(C)) {
                            //DIV不能接立即数，因此需要将此立即数存起来
                            instVec.emplace_back("MOV", "TEMP", C);
                            instVec.emplace_back("DIV", "TEMP", "");
                        }
                        else {
                            instVec.emplace_back("DIV", C, "");
                        }
                        instVec.emplace_back("MOV", R, "AL");
                    }
                    else if (qt.op == OPTIMIZE::OperatorType::ADD) {
                        instVec.emplace_back("ADD", R, C);
                    }
                    else if (qt.op == OPTIMIZE::OperatorType::SUB) {
                        instVec.emplace_back("SUB", R, C);
                    }
                    else if (qt.op == OPTIMIZE::OperatorType::EQUAL || qt.op == OPTIMIZE::OperatorType::GREATER || qt.op == OPTIMIZE::OperatorType::LESS ||
                             qt.op == OPTIMIZE::OperatorType::GREATER_EQUAL || qt.op == OPTIMIZE::OperatorType::LESS_EQUAL || qt.op == OPTIMIZE::OperatorType::NOT_EQUAL) {
                        instVec.emplace_back("CMP", R, C);
                        cmpType = qt.op;
                    }
                    posMap[qt.result] = Position(R, false);
                    RDL[R] = qt.result;
                }
            }
            else if (qt.op == OPTIMIZE::IF) {
                //如果是跳转语句，因为跳转语句需要判断的对象必然刚计算完毕，故可以用标志转移指令进行跳转
                saveR(block.curFun);
                if (cmpType == OPTIMIZE::OperatorType::LESS) {
                    instVec.emplace_back("JAE", "", "");
                    }
                else if (cmpType == OPTIMIZE::OperatorType::GREATER) {
                    instVec.emplace_back("JBE", "", "");
                    }
                else if (cmpType == OPTIMIZE::OperatorType::EQUAL) {
                    instVec.emplace_back("JNE", "", "");
                    }
                sem.push(instVec.size() - 1);//等待回填
            }
            else if (qt.op == OPTIMIZE::ELSE) {
                //同时为跳转指令和跳转目标
                saveR(block.curFun);
                instVec.emplace_back("JMP", "", "");//无条件跳到endif之后，需要返填
                int pos = sem.top();//获取到需要返填的位置
                sem.pop();
                sem.push(instVec.size() - 1);
                instVec[pos].name1 = ("LABEL" + QString::number(labelNow));//将JMP的下一条返填进去
                instVec.emplace_back("", "", "", "LABEL" + QString::number(labelNow) + ":");
                labelNow++;
            }
            else if (qt.op == OPTIMIZE::ENDIF) {
                saveR(block.curFun);
                int pos = sem.top();
                sem.pop();
                instVec[pos].name1 = "LABEL" + QString::number(labelNow);//将JMP的下一条返填进去
                instVec.emplace_back("", "", "", "LABEL" + QString::number(labelNow) + ":");
                labelNow++;
            }
            else if (qt.op == OPTIMIZE::WHILE) {
                //为whileend以后回到开始重新判断条件保存下一条语句位置
                sem.push(instVec.size());
                instVec.emplace_back("", "", "", "LABEL" + QString::number(labelNow) + ":");
                labelNow++;
            }
            else if (qt.op == OPTIMIZE::DO) {
                saveR(block.curFun);//..........
                if (cmpType == OPTIMIZE::LESS) {
                    instVec.emplace_back("JAE", "", "");
                }
                else if (cmpType == OPTIMIZE::GREATER) {
                    instVec.emplace_back("JBE", "", "");
                }
                else if (cmpType == OPTIMIZE::EQUAL) {
                    instVec.emplace_back("JNE", "", "");
                }
                sem.push(instVec.size() - 1);//等待回填
            }
            else if (qt.op == OPTIMIZE::ENDWHILE) {
                saveR(block.curFun);
                instVec.emplace_back("JMP", "", "");//跳回判断部分的语句
                int temp = instVec.size() - 1;//上一行加入的语句的位置
                int pos = sem.top();
                sem.pop();
                instVec[pos].name1 = "LABEL" + QString::number(labelNow);//将JMP的下一条返填进去
                instVec.emplace_back("", "", "", "LABEL" + QString::number(labelNow)+ ":");
                labelNow++;
                pos = sem.top();
                sem.pop();
                instVec[temp].name1 = instVec[pos].label;
            }
            else if (qt.op == OPTIMIZE::PROGRAM) {
                    instVec.emplace_back("", "", "", "MAIN:");
                    instVec.emplace_back("MOV", "AX", "DSEG");
                    instVec.emplace_back("MOV", "DS", "AX");
                    instVec.emplace_back("MOV", "AX", "SSEG");
                    instVec.emplace_back("MOV", "SS", "AX");
                    instVec.emplace_back("MOV", "SI", "0");
                    instVec.emplace_back("LEA", "BP", "DATA");
                }
            else if( qt.op == OPTIMIZE::PROCEDURE) {
                instVec.emplace_back("PROC", "NEAR", "", block.curFun);
                rec= {(int)(instVec.size()-1), 0};
            }
            else if (qt.op == OPTIMIZE::PROCEDURE_END){
                if (block.curFun == "main") {
                    instVec.emplace_back("ENDS", "", "", "CSEG");
                    instVec.emplace_back("END", "MAIN", "");
                }
                else {
                    instVec.emplace_back("ENDP", "", "", block.curFun);
                    rec.end = (int)(instVec.size() - 1);
                    record.push_back(rec);
                }
            }
            /* else if (qt.op == "return") {
                if (block.curFun == "main") {
                    instVec.emplace_back("MOV", "AH", "4CH");
                    instVec.emplace_back("INT", "21H", "");
                    RDL["BL"].clear();
                    RDL["CL"].clear();
                    RDL["DL"].clear();
                }
                else {
                    instVec.emplace_back("MOV", "AL", getAddr(qt.arg1, block.curFun));//将返回值暂存在AL中
                    instVec.emplace_back("RET", "", "");
                }
            } */
            else if (qt.op == OPTIMIZE::CALL) {
                instVec.emplace_back("CALL", qt.arg1, "");
            }
            else if (qt.op == OPTIMIZE::WRITE) {
                saveR(block.curFun);
                instVec.emplace_back("MOV", "DL", getAddr(qt.arg1, block.curFun),"");
                instVec.emplace_back("ADD", "DL", "30H","");
                instVec.emplace_back("MOV", "AH", "02H","");
                instVec.emplace_back("INT", "21H", "","");
            }
            else if (qt.op == OPTIMIZE::RED) {
                saveR(block.curFun);
                instVec.emplace_back("MOV", "AH", "01H","");
                instVec.emplace_back("SUB", "DL", "30H","");
                instVec.emplace_back("INT", "21H", "","");
                instVec.emplace_back("MOV", getAddr(qt.arg1, block.curFun), "AL","");
            }
        }
    }
}

OBJECT::SymbolInfo OBJECT::getSymbolInfo(const QString& name, const QString& curFun) {
    SymbolInfo info;
    info.isIN = false;
    info.pos = -1;
    int level1 = -1;
    int level2 = -1;
    for(const auto& temp:pl0::tablelist)
    {
        if(temp.name==curFun)
        {
            level1 = temp.level;
            break;
        }
    }
    if (level1 == -1) {
        QMessageBox::critical(nullptr, "Error", "getsymbolInfo" + curFun + " not found in symbol table.");
    }
    for (const auto& temp : pl0::tablelist) {
        if (temp.name == name && temp.level <= level1) {
            if(temp.level > level2) {
            info.isIN = true;
            info.pos = temp.address;
            level2 = temp.level;
            }
        }
    }
    return info;
}

QString OBJECT::getAddr(const QString& name, const QString& curFun) {
    if (isNum(name)) {
        return name;
    }
    // 检查是否为数组访问（例如 "arr[5]" 形式）
    if (name.contains("[")) {
        int pos = name.indexOf("[");
        QString arrName = name.left(pos);
        QString index = name.mid(pos + 1, name.length() - pos - 2);

        // 查找数组信息
        for (const auto& array : arrayNames) {
            if (array.name == arrName) {
                // 计算偏移量 = (索引 - 下界) * 元素大小
                int offset = 0;
                if (isNum(index)) {
                    offset = (index.toInt() - array.down) * 1; // 假设每个元素1字节
                }

                // 生成数组元素地址: [数组基址 + 偏移量]
                return "BYTE PTR [" + arrName + " + " + QString::number(offset) + "]";
            }
        }
    }
    if (!posMap[name].isMem) {
        return posMap[name].pos;
    }
    else {
        auto addr = getSymbolInfo(name, curFun);
        if (addr.isIN) {
            return "BYTE PTR [BP+" + QString::number(addr.pos) + "]";
        }
        else {
            QMessageBox::critical(nullptr, "Error", "getaddr " + name + " not found in symbol table.");
            return name;
        }
    }
}

QString OBJECT::findEmpty() {
    if (RDL["BL"].isEmpty())
        return "BL";
    if (RDL["CL"].isEmpty())
        return "CL";
    if (RDL["DL"].isEmpty())
        return "DL";
    return "";
}

//出基本块的时候把所有活跃变量放回内存
void OBJECT::saveR(const QString& curFun) {
    for (const auto& pair : RDL) {
        if (!pair.second.isEmpty()) {
            if (isActive(pair.second, curFun)) {
                posMap[pair.second].isMem = true;
                instVec.emplace_back("MOV", getAddr(pair.second, curFun), pair.first);
                RDL[pair.first].clear();
            } else {
                RDL[pair.first].clear();
            }
        }
    }
}
//目的是给出操作使用的寄存器
void OBJECT::getR(const OPTIMIZE::QuadTuple& qtInput, QString& R, QString& B, QString& C, const QString& curFun)
{
    OPTIMIZE::QuadTuple qt = qtInput;
    //如果B在寄存器中
    if (posMap.find(qt.arg1) != posMap.end() && !posMap[qt.arg1].isMem) {
        //B活跃需另存
        if (qt.isactive1 && (!posMap[qt.arg1].isMem || qt.arg1 != qt.result)) {
            QString emptyR = findEmpty();
            //如果有空寄存器，B存到空寄存器中
            if (!emptyR.isEmpty()) {
                instVec.emplace_back("MOV", emptyR, posMap[qt.arg1].pos);
                RDL[emptyR] = qt.arg1;
                R = getAddr(qt.arg1, curFun); B = emptyR;
                if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
                RDL[posMap[qt.arg1].pos].clear();
                posMap[qt.arg1] = Position(emptyR, false);
            } 
            //如果没有空寄存器，B存到内存中
            else {
                posMap[qt.arg1].isMem = true;
                instVec.emplace_back("MOV", getAddr(qt.arg1, curFun), posMap[qt.arg1].pos);
                R = posMap[qt.arg1].pos; B = getAddr(qt.arg1, curFun);
                if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
                RDL[posMap[qt.arg1].pos].clear();
                posMap[qt.arg1] = Position(qt.arg1, true);
            }
        } 
        //B不活跃，直接抢
        else {
            R = getAddr(qt.arg1, curFun); B = getAddr(qt.arg1, curFun);
            if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
            RDL[posMap[qt.arg1].pos].clear();
            posMap[qt.arg1] = Position(qt.arg1, true);
        }
    }
    //B不在但是C在寄存器中，且op可交换
    else if ((qt.op == OPTIMIZE::OperatorType::ADD || qt.op == OPTIMIZE::OperatorType::MUL || qt.op == OPTIMIZE::OperatorType::EQUAL) &&
             posMap.find(qt.arg2) != posMap.end() && !posMap[qt.arg2].isMem) {
        swap(qt.arg1, qt.arg2);
        swap(qt.isactive1, qt.isactive2);
        //把C和B互换，转化为上一种情况
        if (qt.isactive1 && (!posMap[qt.arg1].isMem || qt.arg1 != qt.result)) {
            QString emptyR = findEmpty();
            if (!emptyR.isEmpty()) {
                instVec.emplace_back("MOV", emptyR, posMap[qt.arg1].pos);
                RDL[emptyR] = qt.arg1;
                R = getAddr(qt.arg1, curFun); B = emptyR;
                if (qt.op != OPTIMIZE::OperatorType::ASSIGN ) C = getAddr(qt.arg2, curFun);
                RDL[posMap[qt.arg1].pos].clear();
                posMap[qt.arg1] = Position(emptyR, false);
            } else {
                posMap[qt.arg1].isMem = true;
                instVec.emplace_back("MOV", getAddr(qt.arg1, curFun), posMap[qt.arg1].pos);
                R = posMap[qt.arg1].pos; B = getAddr(qt.arg1, curFun);
                if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
                RDL[posMap[qt.arg1].pos].clear();
                posMap[qt.arg1] = Position(qt.arg1, true);
            }
        } else {
            R = getAddr(qt.arg1, curFun); B = getAddr(qt.arg1, curFun);
            if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
            RDL[posMap[qt.arg1].pos].clear();
            posMap[qt.arg1] = Position(qt.arg1, true);
        }
    }
    //如果B和C都不在寄存器中，就找一个空的寄存器
    else if (!findEmpty().isEmpty()) {
        QString emptyR = findEmpty();
        R = emptyR; B = getAddr(qt.arg1, curFun);
        if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
    }
    //如果没有空寄存器，强制释放
    else {
        QString obj = "";
        QString objPos;
        for (auto pair : RDL) {
            if (!isActive(pair.second, curFun) && posMap.find(pair.second) != posMap.end() && !posMap[pair.second].isMem) {
                obj = pair.second;
                objPos = pair.first;
                break;
            }
        }
        if (obj.isEmpty()) {
            for (auto pair : RDL) {
                if (posMap.find(pair.second) != posMap.end()) {
                    obj = pair.second;
                    objPos = pair.first;
                    break;
                }
            }
        }
        instVec.emplace_back("MOV", obj, objPos);
        RDL[objPos].clear();
        posMap[obj] = Position(obj, true);
        R = objPos; B = getAddr(qt.arg1, curFun);
        if (qt.op != OPTIMIZE::OperatorType::ASSIGN) C = getAddr(qt.arg2, curFun);
    }
}

bool OBJECT::isActive(const QString& name, QString curFun)
{
    SymbolInfo info = getSymbolInfo(name, curFun);
    if (info.isIN) {
        //如果在符号表中，说明是活跃的
        return true;
    } else {
        //如果不在符号表中，说明不是活跃的
        return false;
    }
}

bool OBJECT::isOperator(const OPTIMIZE::OperatorType& qt)
{
    //检查是否为操作符
    switch (qt) {
        case OPTIMIZE::OperatorType::ASSIGN:
        case OPTIMIZE::OperatorType::ADD:
        case OPTIMIZE::OperatorType::SUB:
        case OPTIMIZE::OperatorType::MUL:
        case OPTIMIZE::OperatorType::DIV:
        case OPTIMIZE::OperatorType::EQUAL:
        case OPTIMIZE::OperatorType::GREATER:
        case OPTIMIZE::OperatorType::LESS:
        case OPTIMIZE::OperatorType::GREATER_EQUAL:
        case OPTIMIZE::OperatorType::LESS_EQUAL:
        case OPTIMIZE::OperatorType::NOT_EQUAL:
            return true;
        default:
            return false;
    }
}

bool OBJECT::isNum(const QString& str)
{
    //检查字符串是否为数字
    for (auto ch : str) {
        if (!isdigit(ch.toLatin1())) return false;
    }
    return !str.isEmpty();
}
