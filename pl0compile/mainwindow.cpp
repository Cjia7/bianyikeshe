#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QScrollArea>
#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    ui->label->setText(outputText);
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
    currentEditor->setProperty("filePath", fileName);  // 存储文件路径
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

    // **改为保存文件对话框**
    QString filename = QFileDialog::getSaveFileName(this, "另存为", QDir::homePath(), "文本文件 (*.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) return;  // 用户取消保存

    // **检查是否已存在文件**
    QFile file(filename);
    if (file.exists()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "文件已存在", "该文件已存在，是否要替换？",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;  // 用户选择不替换
    }

    // **写入文件**
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件");
        return;
    }
    QTextStream out(&file);
    out << currentEditor->toPlainText();
    file.close();
}


void MainWindow::on_actiondsd_triggered()
{
    // 获取当前选项卡
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1) return;  // 没有选项卡则返回

    // 获取当前选项卡的 QWidget
    QWidget *currentTab = ui->tabWidget->widget(currentIndex);

    if (currentTab) {
        // 找到当前选项卡中的 QScrollArea
        QScrollArea *currentScrollArea = currentTab->findChild<QScrollArea*>();
        if (currentScrollArea) {
            // 获取 QScrollArea 的内容部件
            QWidget *currentContent = currentScrollArea->widget();
            if (currentContent) {
                // 创建新的选项卡
                QWidget *newTab = new QWidget;
                QVBoxLayout *layout = new QVBoxLayout(newTab);

                // 创建新的 QScrollArea 并复制属性
                QScrollArea *newScrollArea = new QScrollArea;
                newScrollArea->setGeometry(currentScrollArea->geometry()); // 复制位置
                newScrollArea->setStyleSheet(currentScrollArea->styleSheet()); // 复制样式表
                newScrollArea->setWidgetResizable(currentScrollArea->widgetResizable()); // 复制 widgetResizable 属性

                // 创建新的内容部件并复制属性
                QWidget *newContent = new QWidget;
                newContent->setGeometry(currentContent->geometry()); // 复制位置
                newContent->setStyleSheet(currentContent->styleSheet()); // 复制样式表
                newContent->setFixedWidth(currentContent->width()); // 复制宽度
                newContent->setFixedHeight(currentContent->height()); // 复制高度

                // 在新内容中添加新的 QTextEdit 并复制属性
                if (QTextEdit *currentEditor = currentContent->findChild<QTextEdit*>()) {
                    QTextEdit *newEditor = new QTextEdit;
                    newEditor->setGeometry(currentEditor->geometry()); // 复制位置
                    newEditor->setStyleSheet(currentEditor->styleSheet()); // 复制样式表
                    newEditor->setFixedWidth(currentEditor->width()); // 复制宽度
                    newEditor->setFixedHeight(currentEditor->height()); // 复制高度
                    newContent->setLayout(new QVBoxLayout); // 添加布局
                    newContent->layout()->addWidget(newEditor);
                }

                // 设置新的内容部件到新的 QScrollArea
                newScrollArea->setWidget(newContent);

                // 添加 QScrollArea 到新选项卡
                layout->addWidget(newScrollArea);
                newTab->setLayout(layout);

                // 添加到 TabWidget
                ui->tabWidget->addTab(newTab, "新建窗口");
            }
        }
    }
}


void MainWindow::on_actiond_triggered()
{
    // 获取当前选项卡
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1) return;  // 没有选项卡则返回

    // 获取当前选项卡的 QWidget
    QWidget *currentTab = ui->tabWidget->widget(currentIndex);

    // 在当前选项卡中查找 QTextEdit
    QTextEdit *currentEditor = currentTab->findChild<QTextEdit*>();
    if (!currentEditor) return;  // 如果没有找到 QTextEdit，则返回

    // **检查是否已关联文件**
    QString currentFilePath = currentEditor->property("filePath").toString();

    if (currentFilePath.isEmpty()) {
        // **新文件：弹出 "另存为" 对话框**
        currentFilePath = QFileDialog::getSaveFileName(this, "保存文件", QDir::homePath(), "文本文件 (*.txt);;所有文件 (*.*)");
        if (currentFilePath.isEmpty()) return; // 用户取消保存

        // **存储文件路径**
        currentEditor->setProperty("filePath", currentFilePath);
    }

    // **直接保存（覆盖原文件）**
    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件");
        return;
    }

    QTextStream out(&file);
    out << currentEditor->toPlainText();
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
        ui->label->setText(outputText);
    } else {
        // 关闭当前窗口，打开 assembly 界面
        assembly *assemblyWindow = new assembly();
        this->close();
        assemblyWindow->show();
    }
}

