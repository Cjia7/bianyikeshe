#include "token.h"
#include "ui_token.h"

token::token(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::token)
{
    ui->setupUi(this);
}

token::~token()
{
    delete ui;
}
