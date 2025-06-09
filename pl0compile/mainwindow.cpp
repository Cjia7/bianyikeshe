#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::analyzeText() {
    // 获取 textEdit 里的文本
    QString inputText = ui->textEdit->toPlainText();

    // 创建 token 解析对象
    token tokenizer(nullptr, inputText.toStdString());

    // 执行词法分析
    tokenizer.scan();

    // 获取所有输出信息，包括错误和分析结果
    QString outputText = QString::fromStdString(tokenizer.getOutputText());

    // 显示在 textEdit_2
    ui->textEdit_2->setPlainText(outputText);
}
void MainWindow::on_actions_triggered()
{
    // 获取当前选项卡
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1) return;  // 没有选项卡则返回

    // 获取当前选项卡的 QWidget
    QWidget *currentTab = ui->tabWidget->widget(currentIndex);

    // 在当前选项卡中查找 QTextEdit
    QTextEdit *currentEditor = currentTab->findChild<QTextEdit*>();
    if (!currentEditor) return;  // 如果没有找到 QTextEdit，则返回
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), "文本文件 (*.txt);;所有文件 (*.*)");

    // 检查是否选择了文件
    if (fileName.isEmpty()) return;

    // 读取文件内容
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件");
        return;
    }

    // 读取文本并显示
    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 将内容显示在 `QTextEdit`
    currentEditor->setPlainText(fileContent);
}


void MainWindow::on_actionds_triggered()
{
    // 获取当前选项卡
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1) return;  // 没有选项卡则返回

    // 获取当前选项卡的 QWidget
    QWidget *currentTab = ui->tabWidget->widget(currentIndex);

    // 在当前选项卡中查找 QTextEdit
    QTextEdit *currentEditor = currentTab->findChild<QTextEdit*>();
    if (!currentEditor) return;  // 如果没有找到 QTextEdit，则返回
    QString filename=QFileDialog::getOpenFileName(this,"选择文件",QDir::homePath(),"文本文件 (*.txt;;所有文件 (*.*)");
    if(filename.isEmpty()) return;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,"错误","无法打开文件");
        return;
    }
    QTextStream out(&file);
    out<<ui->textEdit->toPlainText();
    file.close();
}


void MainWindow::on_actiondsd_triggered()
{
    // 获取当前选项卡
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1) return;  // 没有选项卡则返回

    // 获取当前选项卡的 QWidget
    QWidget *currentTab = ui->tabWidget->widget(currentIndex);

    // 在当前选项卡中查找 QTextEdit
    QTextEdit *currentEditor = currentTab->findChild<QTextEdit*>();
    if (!currentEditor) return;  // 如果没有找到 QTextEdit，则返回
    // 创建新的选项卡窗口
    QWidget *newTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(newTab);

    // 创建新的 QTextEdit 作为编辑区
    QTextEdit *newEditor = new QTextEdit;
    newEditor->setPlaceholderText("这里是新建的选项卡");

    layout->addWidget(newEditor);
    newTab->setLayout(layout);
    // 添加到 TabWidget，并设置选项卡标题
    ui->tabWidget->addTab(newTab, "新建窗口");
}


void MainWindow::on_actiond_triggered()
{
    QString filename=QFileDialog::getOpenFileName(this,"选择文件",QDir::homePath(),"文本文件 (*.txt;;所有文件 (*.*)");
    if(filename.isEmpty()) return;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,"错误","无法打开文件");
        return;
    }
    QTextStream out(&file);
    out<<ui->textEdit->toPlainText();
    file.close();
}


void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    QWidget *tab = ui->tabWidget->widget(index);
    ui->tabWidget->removeTab(index);
    delete tab; // 释放内存，避免窗口残留
}


void MainWindow::on_actions_2_triggered()
{

        // 获取 textEdit 的输入文本
    QString inputText = ui->textEdit->toPlainText();

    // 创建词法分析对象
    token tokenizer(nullptr, inputText.toStdString());

    // 运行词法分析
    tokenizer.scan();

    // 获取词法分析输出信息
    QString outputText = QString::fromStdString(tokenizer.getOutputText());



    // 判断是否有错误
    if (tokenizer.gethasError()) {
        QMessageBox::warning(this, "词法分析错误", "代码存在错误，请查看详细信息！");
        // 显示在 textEdit_2
        ui->textEdit_2->setPlainText(outputText);
    } else {
        // 关闭当前窗口，打开 assembly 界面
        assembly *assemblyWindow = new assembly();
        this->close();
        assemblyWindow->show();
    }
}

