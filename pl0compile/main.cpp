#include "mainwindow.h"
#include"QDebug"
#include"qdebug.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qDebug()<<"hello world";
    w.show();
    return a.exec();
}
