#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE

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
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
