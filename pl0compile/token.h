#ifndef TOKEN_H
#define TOKEN_H

#include <QWidget>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
struct Array {
    string name;
    int up;    //数组上届
    int down;   //数组下界
};
extern vector<Array> arrayNames;                //声明为全局变量确保词法和语法分析都能用上

bool isArrayName(const string& str);

namespace Ui {
class token;
}
enum TokenType {
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
    RBR,               // ")"
    ARR,                //"array"
    LBK,               //"["
    RBK,                //"]"
    COL                 //":"
};
extern unordered_map<string, TokenType> tokenMap;
struct Token {
    string name;
    int type;
    int line;
};
class token : public QWidget
{
    Q_OBJECT

public:
    explicit token(QWidget *parent = nullptr,const string& input=" ");
    ~token();
    void scan();
    void printResults();
    string getstring();
    string getOutputText();//存储我cout的部分
    bool gethasError(){return hasError;};
    void handleArrayInVar();
    bool isarrayvar(const string& name);
    bool isArrayDefinitionFollowing(const string& name);
    bool parseArrayBounds(const string& name);
    bool isTypeSpecified();
    void skipWhitespace();
private:
    Ui::token *ui;
    int line = 1;
    size_t pos;
    int line1 = 1;
    string input;
    size_t position = 0;
    vector<Token> tokens;
    bool hasError = false;
    stringstream outputStream;
    std::vector<std::string> keywordTable;
    std::vector<std::string> delimiterTable;
    void initTables() {
        keywordTable = { "program", "procedure", "function", "break", "real", "while",
                        "do", "record", "const", "case", "for", "return", "if", "else", "default" ,"array"};
        delimiterTable = { "=", "<", "<=", ">=", ">", "<>", "+", "-", "*", "/",
                          ":=", ",", ";", ".", "(", ")" ,"[","]",":"};
    }

    int getTokenType(const string& c) {
        if (tokenMap.find(c) != tokenMap.end()) {
            return tokenMap[c];
        }
        return SYMBOL;
    }

    bool isAlpha(char c) {
        return isalpha(static_cast<unsigned char>(c));
    }

    bool isAlphaNum(char c) {
        return isalnum(static_cast<unsigned char>(c));
    }

    string getLongestDelimiter() {
        string longest = "";
        for (int len = 1; len <= 2; ++len) { // 界符最长2字符
            if (position + len > input.length()) break;

            std::string current = input.substr(position, len);
            for (const auto& delim : delimiterTable) {
                if (delim == current) {
                    if (current.length() > longest.length()) {
                        longest = current;
                    }
                }
            }
        }
        return longest;
    }

};

#endif // TOKEN_H
