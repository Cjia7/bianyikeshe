#include <map>
#include <utility>
#include <qfile.h>
#include <QTextStream>
#include <iostream>
#include <set>
#include <QMessageBox>
#include <QString>
#include "pl0.h"
#include "dag.h"

std::vector<OPTIMIZE::BasicBlock> OPTIMIZE::DbasicBlocks; // DAG优化后的基本块数组

void OPTIMIZE::optimize()
{
    loadQuadTuples();    // 加载四元式
    divideBasicBlocks(); // 划分基本块
    for (auto &block : basicBlocks)
        DbasicBlocks.emplace_back(DAGToQuadTuple(optimizeOneBlock(block.basic_block)), block.curFun);
    set_active(); // 设置活跃信息
    // 测试输出
    for (const auto &block : DbasicBlocks)
    {
        int count = 0;
        for (const auto &qt : block.basic_block)
        {
            std::cout << "op:" << OTQ(qt.op).toStdString() << ", arg1:" << qt.arg1.toStdString() << "(活跃:" << qt.isactive1 << ")"
                      << ", arg2:" << qt.arg2.toStdString() << "(活跃:" << qt.isactive2 << ")"
                      << ", result:" << qt.result.toStdString() << "(活跃：" << qt.isactiveR << ")"
                      << std::endl;
        }
        std::cout << ++count<<"当前基本块函数名：" << block.curFun.toStdString() << std::endl;
        std::cout << "------------------------" << std::endl;
    }
    std::cout << "DAG优化完成，基本块数量：" << DbasicBlocks.size() << std::endl;
}

void OPTIMIZE::set_active()
{
    // 遍历第一遍获得所有变量
    for (auto &block : DbasicBlocks)
    {
        activeMap.clear();
        // 初始化活跃信息表
        for (auto &tuple : block.basic_block)
        {
            if (!tuple.result.isEmpty())
            {
                if (activeMap.find(tuple.result) == activeMap.end())
                {
                    if (!isTempName(tuple.result))
                    {
                        activeMap[tuple.result] = true;
                    }
                    else
                    {
                        activeMap[tuple.result] = false;
                    }
                }
            }
            if (!tuple.arg1.isEmpty())
            {
                if (activeMap.find(tuple.arg1) == activeMap.end())
                {
                    if (!isTempName(tuple.arg1))
                    {
                        activeMap[tuple.arg1] = true;
                    }
                    else
                    {
                        activeMap[tuple.arg1] = false;
                    }
                }
            }
            if (!tuple.arg2.isEmpty())
            {
                if (activeMap.find(tuple.arg2) == activeMap.end())
                {
                    if (!isTempName(tuple.arg2))
                    {
                        activeMap[tuple.arg2] = true;
                    }
                    else
                    {
                        activeMap[tuple.arg2] = false;
                    }
                }
            }
        }
        // 逆序标注活跃信息
        for (int i = block.basic_block.size() - 1; i >= 0; --i)
        {
            QuadTuple &tuple = block.basic_block[i];
            if (activeMap.find(tuple.result) != activeMap.end())
            {
                tuple.isactiveR = activeMap[tuple.result];
                activeMap[tuple.result] = false;
            }
            if (activeMap.find(tuple.arg1) != activeMap.end() && activeMap[tuple.arg1])
            {
                tuple.isactive1 = activeMap[tuple.arg1];
                activeMap[tuple.arg1] = true;
            }
            else
            {
                tuple.isactive1 = false;
            }
            if (activeMap.find(tuple.arg2) != activeMap.end() && activeMap[tuple.arg2])
            {
                tuple.isactive2 = true;
                activeMap[tuple.arg2] = true;
            }
            else
            {
                tuple.isactive2 = false;
            }
        }
    }
}

void OPTIMIZE::loadQuadTuples()
{
    QFile file("quadruple.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning("Cannot open quadruple.txt");
        return;
    }
    QTextStream in(&file);
    Rquadtuples.clear();
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        QStringList parts = line.split(',');
        if (parts.size() < 4)
            continue; // 四元式应有4项
        QString opStr = parts[0].trimmed();
        OperatorType op = QTO(opStr);
        if(parts[1] == "_")
        {
            parts[1]=""; // 如果arg1为下划线，则设置为空字符串
        }
        if(parts[2] == "_")
        {
            parts[2]=""; // 如果arg2为下划线，则设置为空字符串
        }
        if(parts[3] == "_")
        {
            parts[3]=""; // 如果result为下划线，则设置为空字符串
        }
        QuadTuple qt(op, parts[1].trimmed(), parts[2].trimmed(), parts[3].trimmed());
        Rquadtuples.push_back(qt);
    }
    file.close();
}

void OPTIMIZE::divideBasicBlocks()
{
    std::set<std::pair<int, QString>> leaders;
    QString curProce;
    QString lastProce;
    for (int i = 0; i < Rquadtuples.size(); i++)
    {
        bool is_in = false;
        if(Rquadtuples[i].op == PROGRAM)
        {
            curProce = Rquadtuples[i].arg1;
            lastProce = curProce;
        }
        if (Rquadtuples[i].op == PROCEDURE)
        {
            lastProce = curProce;
            curProce = Rquadtuples[i].arg1;
            for(const auto &leader : leaders)
            {
                if(leader.first == i)
                {
                    is_in = true;
                }
            }
            if(!is_in)
            {
                leaders.insert(std::make_pair(i, lastProce));
            }
        }
        else if (Rquadtuples[i].op == ENDIF ||Rquadtuples[i].op == IF 
            || Rquadtuples[i].op == DO || Rquadtuples[i].op == ELSE|| Rquadtuples[i].op == PROCEDURE_END|| 
            Rquadtuples[i].op == ENDWHILE)
        {
            // 紧跟在转向语句后面的语句是入口语句
            leaders.insert(std::make_pair(i + 1, curProce));
        }
        else if (Rquadtuples[i].op == WHILE )
        {
            // 转向语句的目标是入口语句
            leaders.insert(std::make_pair(i, curProce));
        }
        if(Rquadtuples[i].op == PROCEDURE_END)
        {
            curProce = lastProce; // 恢复上一个过程名
        }
        else if (Rquadtuples[i].op == CALL )
        {
            leaders.insert(std::make_pair(i, curProce));
            leaders.insert(std::make_pair(i + 1, curProce));
        }
    }
    int start = 0;
    for (auto end : leaders)
    {
        std::vector<QuadTuple> curBlock = {Rquadtuples.begin() + start, Rquadtuples.begin() + end.first};
        if(curBlock.empty())
        {
            continue; // 如果当前块为空，则跳过
        }
        basicBlocks.emplace_back(curBlock, end.second);
        start = end.first;
    }
}

std::vector<OPTIMIZE::QuadTuple> OPTIMIZE::DAGToQuadTuple(std::vector<DAGNode> nodes)
{

    auto getValue = [&](int pos)
    {
        return nodes[pos].mainMark.isEmpty() ? nodes[pos].addMarks[0] : nodes[pos].mainMark;
    };
    std::vector<QuadTuple> ans;
    for (auto node : nodes)
    {
        if (node.isDeleted)
        {
            continue;
        }
        if (node.mainMark.isEmpty() && node.addMarks.empty() && node.left == -1 && node.right == -1)
        {
            ans.emplace_back(node.op, "", "", "");
            continue;
        }
        if (node.mainMark.isEmpty() && node.addMarks.empty())
        {
            ans.emplace_back(node.op, getValue(node.left), "", "");
            continue;
        }
        if (node.left == -1 && node.right == -1)
        {
            for (auto NewVar : node.addMarks)
            {
                if (!isTempName(NewVar))
                {
                    ans.emplace_back(OperatorType::ASSIGN, node.mainMark, "", NewVar);
                }
            }
        }
        else
        {
            ans.emplace_back(node.op, getValue(node.left), getValue(node.right), node.addMarks[0]);
            for (int i = 1; i < node.addMarks.size(); i++)
            {
                if (!isTempName(node.addMarks[i]))
                {
                    ans.emplace_back(OperatorType::ASSIGN, node.addMarks[0], "", node.addMarks[i]);
                }
            }
        }
    }
    return ans;
}

bool OPTIMIZE::isOperator(const OPTIMIZE::OperatorType &op)
{
    return (op == OPTIMIZE::OperatorType::ADD || op == OPTIMIZE::OperatorType::SUB ||
            op == OPTIMIZE::OperatorType::MUL || op == OPTIMIZE::OperatorType::DIV ||
            op == OPTIMIZE::OperatorType::GREATER || op == OPTIMIZE::OperatorType::LESS ||
            op == OPTIMIZE::OperatorType::GREATER_EQUAL || op == OPTIMIZE::OperatorType::LESS_EQUAL ||
            op == OPTIMIZE::OperatorType::EQUAL || op == OPTIMIZE::OperatorType::NOT_EQUAL);
}

std::vector<DAGNode> OPTIMIZE::optimizeOneBlock(std::vector<QuadTuple> quadVector)
{
    std::vector<DAGNode> nodes;
    std::map<QString, int> Map; // 用来记录每个变量对应的结点编号
    // 用来在nodes和defineMap里增加新的叶节点的函数
    auto addLeafNode = [&](QString name)
    {
        DAGNode tempNode(nodes.size(), name);
        Map[name] = nodes.size();
        nodes.push_back(tempNode);
        return nodes.size() - 1;
    };

    // 用来处理与四元式的res字段相关事宜的函数
    auto NewVar = [&](QString res, int n)
    {
        // 如果res已被定义过，则需要将之前的定义删除
        if (Map.find(res) != Map.end())
        {
            int pos = Map[res];
            auto it = find(nodes[pos].addMarks.begin(), nodes[pos].addMarks.end(), res);
            // 非主标记删除
            if (it != nodes[pos].addMarks.end())
            {
                nodes[pos].addMarks.erase(it);
            }
        }
        nodes[n].addMarks.push_back(res);
        Map[res] = n;
        if (!isTempName(res))
        {
            swap(nodes[n].addMarks[0], nodes[n].addMarks[nodes[n].addMarks.size() - 1]);
        }
    };

    for (auto &qt : quadVector)
    {
        int positionOfB = -1;
        int positionOfC = -1;
        int n = 0; // 用来记录赋值语句赋值内容的位置

        if (qt.arg1.isEmpty() && qt.arg2.isEmpty())
        {
            DAGNode tempNode(nodes.size(), qt.op, -1, -1);
            nodes.push_back(tempNode);
            continue;
        }
        if (qt.arg2.isEmpty() && qt.op != OPTIMIZE::OperatorType::ASSIGN)
        {
            if (Map.find(qt.arg1) == Map.end())
            {
                positionOfB = addLeafNode(qt.arg1);
            }
            DAGNode tempNode(nodes.size(), qt.op, Map[qt.arg1], -1);
            nodes.push_back(tempNode);
            continue;
        }
        if (Map.find(qt.arg1) == Map.end())
        {
            positionOfB = addLeafNode(qt.arg1);
        }
        if (qt.op == OPTIMIZE::OperatorType::ASSIGN)
        {
            // 为赋值语句的时候
            n = Map[qt.arg1];
            NewVar(qt.result, n);
        }
        else if (isOperator(qt.op))
        {
            // 如果是双目运算
            if (Map.find(qt.arg2) == Map.end())
            {
                // 如果第二个操作数没定义过
                positionOfC = addLeafNode(qt.arg2);
            }
            if (isNum(nodes[Map[qt.arg2]].mainMark) && isNum(nodes[Map[qt.arg1]].mainMark))
            {
                // 如果两个操作数对应的节点的主标记都是常数
                QString ans = calculateNum(qt.op, nodes[Map[qt.arg1]].mainMark,
                                           nodes[Map[qt.arg2]].mainMark);
                if (Map.find(ans) == Map.end())
                {
                    // 如果算出的ans没有定义过，就将它定义为一个叶子节点
                    n = addLeafNode(ans);
                }
                else
                {
                    n = Map[ans];
                }
                if (positionOfC != -1)
                {
                    // 如果C是处理此次四元式新生成的，则将其删除
                    Map.erase(nodes[positionOfC].mainMark);
                    nodes[positionOfC].isDeleted = true;
                }
                if (positionOfB != -1)
                {
                    Map.erase(nodes[positionOfB].mainMark);
                    nodes[positionOfB].isDeleted = true;
                }
                NewVar(qt.result, n);
            }
            else
            {
                bool isDefined = false;
                // 如果不是两个常数之间的运算，就要考虑这个运算的结果是否已经有了
                for (auto node : nodes)
                {
                    if (Map[qt.arg1] == node.left && Map[qt.arg2] == node.right && node.op == qt.op && !node.isDeleted)
                    {
                        n = node.label;
                        isDefined = true;
                        break;
                    }
                }
                if (!isDefined)
                {
                    // 如果这个之前没计算过，要建立一个新的结点，连接两个操作数
                    DAGNode tempNode(nodes.size(), qt.op, Map[qt.arg1], Map[qt.arg2]);
                    n = nodes.size();
                    nodes.push_back(tempNode);
                }
                NewVar(qt.result, n);
            }
        }
    }
    return nodes;
}

bool OPTIMIZE::is_rukou(OperatorType op)
{
    return (op == IF || op == WHILE || op == DO || op == CALL || op == PROGRAM || op == PROCEDURE);
}
bool OPTIMIZE::is_zhuanyi(OperatorType op)
{
    return (op == ELSE || op == ENDIF || op == ENDWHILE);
}
bool OPTIMIZE::is_ting(OperatorType op)
{
    return (op == WRITE || op == RED || op == VARDEF);
}
// 判断是否是临时变量名,即是否在符号表中
bool OPTIMIZE::isTempName(const QString &name)
{
    if (name[0] == 't')
        return true; // 如果是以't'开头，则认为是临时变量名
    return false;    // 否则不是临时变量名
    /* for(const auto& temp: pl0::tablelist) {
        if(temp.name == name) {
            return false; // 如果是变量或常量名，则不是临时变量
        }
    }
    return true; */
}

bool OPTIMIZE::isNum(const QString &str)
{
    bool ok = false;
    str.toInt(&ok);
    return ok;
}

// 计算两个QString数字字符串的结果，返回QString
QString OPTIMIZE::calculateNum(const OperatorType &op, const QString &left, const QString &right)
{
    int l = left.toInt();
    int r = right.toInt();
    int result = 0;
    if (op == OPTIMIZE::OperatorType::ADD)
        result = l + r;
    else if (op == OPTIMIZE::OperatorType::SUB)
        result = l - r;
    else if (op == OPTIMIZE::OperatorType::MUL)
        result = l * r;
    else if (op == OPTIMIZE::OperatorType::DIV)
        result = (r != 0) ? l / r : 0;
    else if (op == OPTIMIZE::OperatorType::LESS)
        result = (l < r) ? 1 : 0;
    else if (op == OPTIMIZE::OperatorType::GREATER)
        result = (l > r) ? 1 : 0;
    else if (op == OPTIMIZE::OperatorType::EQUAL)
        result = (l == r) ? 1 : 0;
    return QString::number(result);
}

OPTIMIZE::OperatorType OPTIMIZE::QTO(const QString &str)
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
        {"PROCEDURE_BEGIN", PROCEDURE_BEGIN},
        {"PROCEDURE_END", PROCEDURE_END},
        {"WRITE", WRITE},
        {"RED", RED},
        {"VARDEF", VARDEF}};
    if (map.contains(str))
        return map.value(str);
    else
    {
        QMessageBox::critical(nullptr, "操作符错误", "Invalid operator type: " + str);
        return ASSIGN;
    }
}

QString OPTIMIZE::OTQ(const OperatorType &op)
{
    static const QHash<OperatorType, QString> map = {
        {ASSIGN, "ASSIGN"},
        {ADD, "ADD"},
        {SUB, "SUB"},
        {MUL, "MUL"},
        {DIV, "DIV"},
        {IF, "IF"},
        {ELSE, "ELSE"},
        {ENDIF, "ENDIF"},
        {WHILE, "WHILE"},
        {DO, "DO"},
        {ENDWHILE, "ENDWHILE"},
        {EQUAL, "EQUAL"},
        {GREATER, "GREATER"},
        {LESS, "LESS"},
        {GREATER_EQUAL, "GREATER_EQUAL"},
        {LESS_EQUAL, "LESS_EQUAL"},
        {NOT_EQUAL, "NOT_EQUAL"},
        {CALL, "CALL"},
        {PROGRAM, "PROGRAM"},
        {PROCEDURE, "PROCEDURE"},
        {PROCEDURE_BEGIN, "PROCEDURE_BEGIN"},
        {PROCEDURE_END, "PROCEDURE_END"},
        {WRITE, "WRITE"},
        {RED, "RED"},
        {VARDEF, "VARDEF"}};
    return map.value(op);
}
