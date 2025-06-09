#include "token.h"
#include "ui_token.h"
#include <fstream>
unordered_map<string, TokenType> tokenMap = {
    {"program", PROGRAM}, {"begin", BEGIN}, {"end", END}, {"if", IF}, {"then", THEN},
    {"else", ELSE}, {"const", CON}, {"procedure", PROCEDURE}, {"var", VAR}, {"do", DO},
    {"while", WHILE}, {"call", CALL}, {"read", READ}, {"write", WRITE}, {"repeat", REPEAT},
    {"odd", ODD}, {"=", EQU}, {"<", LES}, {"<=", LESE}, {">=", LARE}, {">", LAR},
    {"<>", NEQU}, {"+", ADD}, {"-", SUB}, {"*", MUL}, {"/", DIV}, {":=", CEQU},
    {",", COMMA}, {";", SEMIC}, {".", POI}, {"(", LBR}, {")", RBR}
};
token::token(QWidget *parent,const string& input)
    : QWidget(parent)
    , ui(new Ui::token),input(input)
{
    ui->setupUi(this);
    initTables();
}


token::~token()
{
    delete ui;
}
void token::scan(){
    while (position < input.length()) {
        char c = input[position];

        // 统一处理空白字符（包括换行符）
        if (isspace(c)) {
            if (c == '\n') line++;
            position++;
            continue;
        }

        // 标识符和关键字
        if (isAlpha(c)) {
            size_t start = position;
            position++;
            while (position < input.length() && isAlphaNum(input[position])) {
                position++;
            }

            string name = input.substr(start, position - start);
            // 检查标识符长度
            if (name.length() > 10) {
                outputStream << "Identifier too long, line " << line << endl;
                hasError = true;
            }
            // 检查是否为关键字
            else if (tokenMap.find(name) != tokenMap.end()) {
                tokens.push_back({ name, tokenMap[name], line });
            }
            // 否则作为标识符
            else {
                tokens.push_back({ name, SYMBOL, line });
            }
            continue;
        }

        // 整型常量
        if (isdigit(c)) {
            size_t start = position;
            // 只读取数字
            while (position < input.length() && isdigit(input[position])) {
                position++;
            }
            if (isalpha(input[position])) {
                outputStream << "Wrong identifier, line " << line << endl;
                hasError = true;
            }
            string num = input.substr(start, position - start);
            if (num.size() > 14) {
                outputStream << "Number too long, line " << line << endl;
                hasError = true;
            }
            else {
                tokens.push_back({ num, CONST, line });
            }
            continue;
        }

        // 界符处理
        string delim = getLongestDelimiter();
        if (!delim.empty()) {
            tokens.push_back({ delim, tokenMap[delim], line });
            position += delim.length();
            continue;
        }

        // 未识别的字符
        outputStream << "Invalid character: '" << c << "', line " << line << endl;
        hasError = true;
        position++;
    }
    if (hasError) {
        std::cout << "ERROR" << endl;
        return;
    }
    ofstream file("word analysis.txt");
    if (!file.is_open()) {
        cout << "can't open file" << endl; // 文件打开失败
    }
    for (const auto& token : tokens) {
        file<< token.name << "," << token.type << "#" << token.line << "\n";
    }
}
string token::getstring(){
    string input;
    string line;
    ifstream inputFile("input.txt"); // 替换为您的输入文件名
    if (!inputFile.is_open()) {
        cerr << "无法打开输入文件" << endl;
        return NULL;
    }

    while (getline(inputFile, line)) {
        // 保留所有行，包括空行
        input += line + '\n';
    }
    inputFile.close();
    return input;
}

string token::getOutputText() {//公用调用输出部分
    return outputStream.str();
}
