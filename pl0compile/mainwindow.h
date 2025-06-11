#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <assembly.h>
#include <token.h>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
QT_BEGIN_NAMESPACE
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
    void analyzeText();//进行词法分析
private slots:
    void on_actions_triggered();

    void on_actionds_triggered();

    void on_actiondsd_triggered();

    void on_actiond_triggered();

    void on_tabWidget_tabCloseRequested(int index);

    void on_actions_2_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
