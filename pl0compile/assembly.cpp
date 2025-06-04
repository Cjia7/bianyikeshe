#include "assembly.h"
#include "ui_assembly.h"

assembly::assembly(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::assembly)
{
    ui->setupUi(this);
}

assembly::~assembly()
{
    delete ui;
}
