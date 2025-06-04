#ifndef TOKEN_H
#define TOKEN_H

#include <QWidget>

namespace Ui {
class token;
}

class token : public QWidget
{
    Q_OBJECT

public:
    explicit token(QWidget *parent = nullptr);
    ~token();

private:
    Ui::token *ui;
};

#endif // TOKEN_H
