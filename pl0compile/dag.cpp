#include <map>
#include <qfile.h>
#include <QTextStream>
#include <QDebug>
#include "dag.h"

void OPTIMIZE::optimize()
{
    loadQuadTuples(); //加载四元式
    divideBasicBlocks(); //划分基本块
    for(auto& block: basicBlocks) 
        DbasicBlocks.push_back(DAGToQuadTuple(optimizeOneBlock(block)));
    //测试输出
    for(const auto& block: DbasicBlocks) {
        for(const auto& qt: block) {
            qDebug() << "op:" << qt.op << ", arg1:" << qt.arg1 << ", arg2:" << qt.arg2 << ", result:" << qt.result;
        }
    }
}

void OPTIMIZE::loadQuadTuples() {
    QFile file("quadruple.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open quadruple.txt");
        return;
    }
    QTextStream in(&file);
    Rquadtuples.clear();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split(',');
        if (parts.size() < 4) continue; // 四元式应有4项
        QString opStr = parts[0].trimmed();
        OperatorType op= QTO(opStr);
        QuadTuple qt(op, parts[1].trimmed(), parts[2].trimmed(), parts[3].trimmed());
        Rquadtuples.push_back(qt);
    }
    file.close();
}

void OPTIMIZE::divideBasicBlocks() {
    for(int i=0; i < Rquadtuples.size(); i++) 
        if (is_rukou(Rquadtuples[i].op)) {
            std::vector<QuadTuple> temp;
            temp.push_back(Rquadtuples[i]);
            while (i < Rquadtuples.size() ) {
                i++;
                if(is_rukou(Rquadtuples[i].op))
                    break;
                temp.push_back(Rquadtuples[i]);
                if (is_zhuanyi(Rquadtuples[i].op)|| is_ting(Rquadtuples[i].op)){
                    i++;
                    break;
                }
            }
            basicBlocks.push_back(temp);
            continue;
        }
}

std::vector<OPTIMIZE::QuadTuple> OPTIMIZE::DAGToQuadTuple(std::vector<DAGNode> nodes)
{

    auto getValue = [&](int pos) {
        return nodes[pos].mainMark.isEmpty() ? nodes[pos].addMarks[0] : nodes[pos].mainMark;
    };
    std::vector<QuadTuple> ans;
    for(auto node: nodes) {
        if(node.isDeleted) {
            continue;
        }
        if(node.left == -1 && node.right == -1) {
            for(auto addMark: node.addMarks) {
                if(!isTempName(addMark)) {
                    ans.emplace_back(OperatorType::ASSIGN, node.mainMark, "__", addMark);
                }
            }
        } else {
            ans.emplace_back(node.op, getValue(node.left), getValue(node.right), node.addMarks[0]);
            for(int i = 1; i < node.addMarks.size(); i++) {
                if(!isTempName(node.addMarks[i])) {
                    ans.emplace_back(OperatorType::ASSIGN, node.addMarks[0], "__", node.addMarks[i]);
                }
            }
        }
    }
    return ans;
}

std::vector<DAGNode> OPTIMIZE::optimizeOneBlock(std::vector<QuadTuple> quadVector)
{
    std::vector<DAGNode> nodes;
    std::map<QString, int> defineMap;
    //用来在nodes和defineMap里增加新的叶节点的函数, 返回插入的元素的位置的迭代器
    auto addLeafNode = [&](QString name) {
        DAGNode tempNode(nodes.size(), name);
        defineMap[name] = nodes.size();
        nodes.push_back(tempNode);
        return nodes.size() - 1;
    };

    //用来处理与四元式的res字段相关事宜的函数
    auto defineRes = [&](QString res, int n) {
        if(defineMap.find(res) != defineMap.end()) {
            //res未定义过节点，将res附加在位置为n的附加标记上
            int pos = defineMap[res];
            //先删除已有的附加标记
            if (true) {
                //如果不是叶子节点才删除
                auto it = find(nodes[pos].addMarks.begin(), nodes[pos].addMarks.end(), res);
                if(it != nodes[pos].addMarks.end()) {
                    nodes[pos].addMarks.erase(it);
                }
            }
        }
        nodes[n].addMarks.push_back(res);
        defineMap[res] = n;
        if(!isTempName(res)) {
            swap(nodes[n].addMarks[0], nodes[n].addMarks[nodes[n].addMarks.size() - 1]);
        }
    };

    for(auto& qt: quadVector) {
        int positionOfB = -1;
        int positionOfC = -1;

        int n = 0;//用来记录赋值语句赋值内容的位置
        if(defineMap.find(qt.arg1) == defineMap.end()) {
            positionOfB = addLeafNode(qt.arg1);
        }
        if(qt.op == OPTIMIZE::OperatorType::ASSIGN) {
            //为赋值语句的时候
            n = defineMap[qt.arg1];
            defineRes(qt.result, n);
        } else if (qt.op == OPTIMIZE::OperatorType::ADD || qt.op == OPTIMIZE::OperatorType::SUB || qt.op == OPTIMIZE::OperatorType::MUL
             || qt.op == OPTIMIZE::OperatorType::DIV || qt.op == OPTIMIZE::OperatorType::GREATER || qt.op == OPTIMIZE::OperatorType::LESS
             || qt.op == OPTIMIZE::OperatorType::GREATER_EQUAL || qt.op == OPTIMIZE::OperatorType::LESS_EQUAL|| qt.op == OPTIMIZE::OperatorType::EQUAL
             || qt.op == OPTIMIZE::OperatorType::NOT_EQUAL) {
            //如果是双目运算
            if(defineMap.find(qt.arg2) == defineMap.end()) {
                //如果第二个操作数没定义过
                positionOfC = addLeafNode(qt.arg2);
            }
            if(isNum(nodes[defineMap[qt.arg2]].mainMark) && isNum(nodes[defineMap[qt.arg1]].mainMark)) {
                //如果两个操作数对应的节点的主标记都是常数
                QString ans = calculateNum(qt.op, nodes[defineMap[qt.arg1]].mainMark,
                                          nodes[defineMap[qt.arg2]].mainMark);
                if(defineMap.find(ans) == defineMap.end()) {
                    //如果算出的ans没有定义过，就将它定义为一个叶子节点
                    n = addLeafNode(ans);
                } else {
                    n = defineMap[ans];
                }
                if(positionOfC != -1) {
                    //如果C是处理此次四元式新生成的，则将其删除
                    defineMap.erase(nodes[positionOfC].mainMark);
                    nodes[positionOfC].isDeleted = true;
                }
                if(positionOfB != -1) {
                    defineMap.erase(nodes[positionOfB].mainMark);
                    nodes[positionOfB].isDeleted = true;
                }
                defineRes(qt.result, n);
            } else {
                bool isDefined = false; //如果不是两个常数之间的运算，就要考虑这个运算的结果是否已经有了
                for(auto node: nodes) {
                    if(defineMap[qt.arg1] == node.left && defineMap[qt.arg2] == node.right && node.op == qt.op
                       && !node.isDeleted) {
                        n = node.label;
                        isDefined = true;
                        break;
                    }
                }
                if(!isDefined) {
                    //如果这个之前没计算过，要建立一个新的结点，连接两个操作数
                    DAGNode tempNode(nodes.size(), qt.op, defineMap[qt.arg1], defineMap[qt.arg2]);
                    n = nodes.size();
                    nodes.push_back(tempNode);
                }
                defineRes(qt.result, n);
            }
        }
    }
    return nodes;
}

bool OPTIMIZE::is_rukou(OperatorType op) {
    return (op == IF || op == WHILE || op == DO || op == CALL || op == PROGRAM || op == PROCEDURE);
}
bool OPTIMIZE::is_zhuanyi(OperatorType op) {
    return (op == ELSE || op == ENDIF || op == ENDWHILE || op == ENDPROC);
}
bool OPTIMIZE::is_ting(OperatorType op) {
    return (op == WRITE || op == RED || op == VARDEF || op == PRODEF);
}
bool OPTIMIZE::isTempName(const QString& name) {
    /* */
    return true;
}

bool OPTIMIZE::isNum(const QString& str) {
    bool ok = false;
    str.toInt(&ok);
    return ok;
}

// 计算两个QString数字字符串的结果，返回QString
QString OPTIMIZE::calculateNum(const OperatorType& op, const QString& left, const QString& right) {
    int l = left.toInt();
    int r = right.toInt();
    int result = 0;
    if (op == OPTIMIZE::OperatorType::ADD) result = l + r;
    else if (op == OPTIMIZE::OperatorType::SUB) result = l - r;
    else if (op == OPTIMIZE::OperatorType::MUL) result = l * r;
    else if (op == OPTIMIZE::OperatorType::DIV) result = (r != 0) ? l / r : 0;
    else if (op == OPTIMIZE::OperatorType::LESS) result = (l < r) ? 1 : 0;
    else if (op == OPTIMIZE::OperatorType::GREATER) result = (l > r) ? 1 : 0;
    else if (op == OPTIMIZE::OperatorType::EQUAL) result = (l == r) ? 1 : 0;
    return QString::number(result);
}

OPTIMIZE::OperatorType OPTIMIZE::QTO(const QString& str)
{

    static const QHash<QString, OperatorType> map = {
        {"ASSIGN", ASSIGN},
        {"ADD", ADD},
        {"SUB", SUB},
        {"MUL", MUL},
        {"DIV", DIV},
        {"IF", IF},
        {"ELSE", ELSE},
        {"ENDIF", ENDIF},
        {"WHILE", WHILE},
        {"DO", DO},
        {"ENDWHILE", ENDWHILE},
        {"EQUAL", EQUAL},
        {"GREATER", GREATER},
        {"LESS", LESS},
        {"GREATER_EQUAL", GREATER_EQUAL},
        {"LESS_EQUAL", LESS_EQUAL},
        {"NOT_EQUAL", NOT_EQUAL},
        {"CALL", CALL},
        {"PROGRAM", PROGRAM},
        {"PROCEDURE", PROCEDURE},
        {"ENDPROC", ENDPROC},
        {"WRITE", WRITE},
        {"RED", RED},
        {"VARDEF", VARDEF},
        {"PRODEF", PRODEF}
    };
    return map.value(str, ASSIGN); 
}
