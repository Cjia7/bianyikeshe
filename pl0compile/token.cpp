#include "token.h"
#include "ui_token.h"
#include <fstream>
unordered_map<string, TokenType> tokenMap = {
    {"program", PROGRAM}, {"begin", BEGIN}, {"end", END}, {"if", IF}, {"then", THEN},
    {"else", ELSE}, {"const", CON}, {"procedure", PROCEDURE}, {"var", VAR}, {"do", DO},
    {"while", WHILE}, {"call", CALL}, {"read", READ}, {"write", WRITE}, {"repeat", REPEAT},
    {"odd", ODD}, {"=", EQU}, {"<", LES}, {"<=", LESE}, {">=", LARE}, {">", LAR},
    {"<>", NEQU}, {"+", ADD}, {"-", SUB}, {"*", MUL}, {"/", DIV}, {":=", CEQU},
    {",", COMMA}, {";", SEMIC}, {".", POI}, {"(", LBR}, {")", RBR},{"array",ARR},{"[",LBK},{"]",RBK},{":",COL}
};
token::token(QWidget *parent,const string& input)
    : QWidget(parent)
    , ui(new Ui::token),input(input)
{
    ui->setupUi(this);
    initTables();
}

vector<Array> arrayNames;
bool isArrayName(const string& str) {//直接区分是否为大小写
    for (const auto& array : arrayNames) {
        // 直接比较字符串（严格区分大小写）
        if (array.name == str) {
            return true;
        }
    }
    return false;
}
token::~token()
{
    delete ui;
}
void token::handleArrayInVar(){
    line1 = line;
    pos = position-3;
    size_t varPos = pos;
    if (input.substr(varPos, 3) == "var") { // 匹配var关键字
        pos += 3;
        skipWhitespace(); // 跳过空白

        while (pos < input.length()) {
            if (isAlpha(input[pos])) { // 提取标识符（可能是数组名）
                size_t nameStart = pos;
                while (pos < input.length() && isAlphaNum(input[pos])) pos++;
                string name = input.substr(nameStart, pos - nameStart);

                skipWhitespace();
                if (isArrayDefinitionFollowing(name)) { // 检查是否为数组定义
                    // 存储数组名
                }

                // 处理逗号分隔的多个变量/数组
                if (pos < input.length() && input[pos] == ',') {
                    pos++;
                    skipWhitespace();
                    continue;
                }
                // 处理声明结束符（分号或冒号）
                if (input[pos] == ';') break;
            }
            pos++;
        }
    }
}
bool token::isarrayvar(const string& name) {
    pos = position;
    bool current = false;

    if (isArrayName(name)) {
        size_t start = pos - name.length();

        for (const auto& Token : tokens) {
            if (Token.name == name) {
                current = true;
            }
        }
        if(current){
            if (input[pos] == '[') {
                pos++;
                if (isdigit(input[pos])) {
                    size_t start1 = position;
                    // 只读取数字
                    while (pos < input.length() && isdigit(input[pos])) {
                        pos++;
                    }
                    int digit2 = 0;
                    int digit1 = stoi(input.substr(start1+1, pos - start1));

                    if (input.substr(start1, pos - start1).size() > 14) {
                        cout << "Number too long, line " << line << endl;
                        hasError = true;
                        return false;

                    }
                    for (const auto& Array : arrayNames) {
                        if (Array.name == name) {
                            digit2 = Array.up;
                        }
                    }
                    if (digit1 <= digit2) {
                        if (input[pos] == ']') {
                            string name1 = input.substr(start, pos - start + 1);
                            tokens.push_back({ name1,SYMBOL,line });
                            position = pos;
                            position++;
                            return true;
                        }
                        else {
                            outputStream << "第" << line <<"行缺少结构:"<<"]"<< endl;
                            hasError = true;
                            return false;
                        }
                    }
                    else {
                       outputStream << "第" << line <<"行超出范围:"<<"数组数字"<< endl;
                        hasError = true;
                        return false;
                    }
                }
                else {
                   outputStream << "第" << line <<"行错误的数组结构:"<< endl;
                    hasError = true;
                    return false;
                }
            }
            else {
               outputStream << "第" << line <<"行错误的数组结构:"<<"["<< endl;
                hasError = true;
                return false;
            }
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}
// 检查当前位置后是否跟有数组定义结构（: array [..] of int）
bool token::isArrayDefinitionFollowing(const string& name) {
    skipWhitespace();
    if (pos < input.length() && input[pos] == ':') { // 数组定义以冒号开头
        pos++;
        skipWhitespace();

        if (input.substr(pos, 5) == "array") { // 匹配array关键字
            pos += 5;
            skipWhitespace();
            if (input[pos] == '[' && parseArrayBounds(name) && input[pos] == ']') { // 检查一维数组边界
                pos++;
                skipWhitespace();
                if (input.substr(pos, 2) == "of" && isTypeSpecified()) { // 检查of类型
                    return true;
                }
                else {
                    hasError = true;
                    outputStream << "第" << line <<"行错误的数组类型"<< endl;
                }
            }
            else {
                hasError = true;
                outputStream << "第" << line <<"行错误的数组定义"<< endl;
            }
        }
        else {
            hasError = true;
            outputStream << "第" << line <<"行缺少:"<<"array"<< endl;
        }
    }
    // 恢复位置（非数组定义）
    return false;
}

bool token::parseArrayBounds(const string& name) {
    pos++; // 跳过'['
    skipWhitespace();
    int up;
    int down;
    // 下界检查（带花括号）
    if (!isdigit(input[pos])) {

        outputStream << "第" << line <<"行没有数字"<< endl;
        return false;
    }
    size_t start = pos;
    while (pos < input.length() && isdigit(input[pos])) {
        pos++;
    }
    string digit= input.substr(start, pos - start);
    down = stoi(digit);
    skipWhitespace();

    // ".." 检查（带花括号）
    if (pos + 1 >= input.length() || input[pos] != '.' || input[pos + 1] != '.') {
       outputStream << "第" << line <<"行错误的数字结构"<< endl;
        return false;
    }
    pos += 2;
    skipWhitespace();

    // 上界检查（带花括号）
    if (!isdigit(input[pos])) {
       outputStream << "第" << line <<"行没有正确的[]"<< endl;
        return false;
    }
    start = pos;
    while (pos < input.length() && isdigit(input[pos])) {
        pos++;
    }
    digit = input.substr(start, pos - start);
    up = stoi(digit);
    arrayNames.push_back({ name,up,down });
    skipWhitespace();

    // 结束符检查（带花括号）
    if (pos >= input.length() || input[pos] != ']') {
        outputStream << "第" << line <<"行错误的数组结构"<< endl;
        return false;
    }

    return true;
}
// 检查类型是否为int（简化处理）
bool token::isTypeSpecified() {
    pos += 2;
    skipWhitespace();
    if (input.substr(pos, 3) == "int") {
        pos += 3;
        skipWhitespace();
        return true;
    }
    return false;
}

// 跳过空白字符
void token::skipWhitespace() {
    while (pos < input.length() && isspace(input[pos])) {
        if (input[pos++] == '\n') line1++;
    }
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

                if (tokenMap[name] == VAR) { // 仅当识别到var关键字时调用
                    handleArrayInVar();
                }
            }
            // 否则作为标识符
            else {
                if (isarrayvar(name)) {
                    continue;
                }
                else {
                    tokens.push_back({ name, SYMBOL, line });
                }
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
                outputStream << "第" << line <<"行错误的定义"<< endl;
                hasError = true;
            }
            string num = input.substr(start, position - start);
            if (num.size() > 14) {
                outputStream << "第" << line <<"行数字超出范围"<< endl;
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
        outputStream << "第" << line <<"行错误的单词:"<<c<< endl;
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
        file<< token.name << "$" << token.type << "%" << token.line << "\n";
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
