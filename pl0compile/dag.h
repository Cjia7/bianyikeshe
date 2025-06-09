#ifndef DAG_H
#define DAG_H
#include <QString>
#include <vector>

class DAGNode;//前向声明
class OPTIMIZE
{
public:
    enum OperatorType
    {
        ASSIGN,
        ADD,
        SUB,
        MUL,
        DIV,

        IF,
        ELSE,
        ENDIF,
        WHILE,
        DO,
        ENDWHILE,

        EQUAL,
        GREATER,
        LESS,
        GREATER_EQUAL,
        LESS_EQUAL,
        NOT_EQUAL,

        CALL,
        PROGRAM,
        PROCEDURE,
        ENDPROC,

        WRITE,
        RED,

        VARDEF,
        PRODEF
    };

    struct QuadTuple
    {
    OperatorType op;
    QString arg1;
    QString arg2;
    QString result;

    QuadTuple(OperatorType op, const QString& arg1, const QString& arg2, const QString& result)
        : op(op), arg1(arg1), arg2(arg2), result(result) {}
    };

public:
    OPTIMIZE();
    ~OPTIMIZE();
    void optimize();

private:
    std::vector<QuadTuple> Rquadtuples;
    std::vector<std::vector<QuadTuple>> basicBlocks; //基本块
    std::vector<std::vector<QuadTuple>> DbasicBlocks; //DAG优化后的基本块
private:
    void loadQuadTuples();
    void divideBasicBlocks();//划分基本快
    bool is_rukou(OperatorType);
    bool is_zhuanyi(OperatorType);
    bool is_ting(OperatorType);
    OperatorType QTO(const QString& str);
    std::vector<DAGNode> optimizeOneBlock(std::vector<QuadTuple> quadVector);
    std::vector<QuadTuple> DAGToQuadTuple(std::vector<DAGNode> nodes);
    
private:
    bool isTempName(const QString &name);
    bool isNum(const QString& str);
    QString calculateNum(const OperatorType&op, const QString &left, const QString &right);
};
class DAGNode
{
public:
    //用于构造叶结点的构造函数
    DAGNode(int label, QString mainMark) : label(label), mainMark(mainMark), left(-1),
                                           right(-1), addMarks(), isDeleted(false) {};
    //用于构造非叶节点的构造函数
    DAGNode(int label, OPTIMIZE::OperatorType op, int left, int right) : label(label), mainMark(""), op(op), left(left),
                                           right(right), addMarks(), isDeleted(false) {};
    DAGNode() = default;

    int label;
    OPTIMIZE::OperatorType op;
    QString mainMark;
    std::vector<QString> addMarks;
    int left;
    int right;
    bool isDeleted;
};

#endif // DAG_H