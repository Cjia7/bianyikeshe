#include "pl0.h"
#include "ui_pl0.h"

pl0::pl0(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pl0)
{
    ui->setupUi(this);
}

pl0::~pl0()
{
    delete ui;
}
