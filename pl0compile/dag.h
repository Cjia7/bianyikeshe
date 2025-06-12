#ifndef DAG_H
#define DAG_H
#include <QString>
#include <vector>
#include <map>

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
        PROCEDURE_BEGIN,
        PROCEDURE_END,

        WRITE,
        RED,

        VARDEF
    };

    struct QuadTuple
    {
    OperatorType op;
    QString arg1;
    bool isactive1 = true;
    QString arg2;
    bool isactive2 = true;
    QString result;
    bool isactiveR = true;

    QuadTuple(OperatorType op, const QString& arg1, const QString& arg2, const QString& result)
        : op(op), arg1(arg1), arg2(arg2), result(result) {}
    };

    struct BasicBlock
    {
        std::vector<QuadTuple> basic_block; //存储四元式
        QString curFun; //当前基本块所属的函数名
        BasicBlock(std::vector<QuadTuple> block,QString fun) : basic_block(block), curFun(fun) {}
    };

public:
    OPTIMIZE()=default;
    OPTIMIZE(int){
        DbasicBlocks.clear();
        optimize();
    }
    ~OPTIMIZE()=default;
    void optimize();
    void set_active();
    static std::vector<BasicBlock> DbasicBlocks; //DAG优化后的基本块数组
    QString OTQ(const OperatorType &op);
private:
    std::vector<QuadTuple> Rquadtuples;
    std::vector<BasicBlock> basicBlocks; //基本块数组
    std::map<QString, bool> activeMap;
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
    bool isOperator(const OPTIMIZE::OperatorType &op);
    bool isTempName(const QString &name);
    bool isNum(const QString& str);
    QString calculateNum(const OperatorType&op, const QString &left, const QString &right);
};
class DAGNode
{
public:
    //用于构造叶结点的构造函数
    DAGNode(int label, QString mainMark) : label(label), op(OPTIMIZE::ASSIGN),mainMark(mainMark), left(-1),
                                           right(-1), addMarks(), isDeleted(false) {};
    //用于构造非叶节点的构造函数
    DAGNode(int label, OPTIMIZE::OperatorType op, int left, int right) : label(label), mainMark(""), op(op), left(left),
                                           right(right), addMarks(), isDeleted(false) {};
    DAGNode() = default;
    ~DAGNode() = default;
    int label;//结点序号
    OPTIMIZE::OperatorType op;//结点操作符
    QString mainMark;//主标记
    std::vector<QString> addMarks;//附加标记
    int left;//左孩子编号
    int right;//右孩子编号
    bool isDeleted;//是否被删除
};

#endif // DAG_H