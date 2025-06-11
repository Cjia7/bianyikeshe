#include "assembly.h"
#include "ui_assembly.h"
#include <QFile>         // 处理文件读写
#include <QTextStream>   // 处理文本流
#include <QProcess>      // 启动外部进程（用于运行 VSCode）
#include <QMessageBox>   // 弹出错误警告框
#include <QDir>           // 目录操作
#include <QDateTime>      // 日期时间（用于生成唯一文件名）
#include <QStandardPaths> // 获取系统标准路径（如临时目录）
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

// 保存代码到临时文件，返回文件路径
QString assembly::saveCodeToTempFile(const QString &code, const QString &fileExtension) {
    // 创建唯一临时文件名（避免冲突）
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFileName = QDir(tempDir).filePath(QString("temp_code_%1.%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(fileExtension));

    // 写入代码到文件
    QFile file(tempFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << code;
        file.close();
        return tempFileName;
    } else {
        QMessageBox::critical(this, "错误", "无法保存临时文件:\n" + file.errorString());
        return "";
    }
}
void assembly::on_pushButton_4_clicked()
{


    QString code = ui->textEdit->toPlainText();
    qDebug()<<code;
    QString fileExtension = "asm";

    // 保存代码到临时文件
    QString filePath = saveCodeToTempFile(code, fileExtension);
    if (filePath.isEmpty()) return;

    // 构建编译和运行命令
    QString compileCmd, runCmd, combinedCmd;

    // 根据操作系统选择不同的命令

    // Windows 平台命令
    compileCmd = QString("nasm -f win32 \"%1\" -o \"%2\\temp.obj\" && gcc -m32 \"%2\\temp.obj\" -o \"%2\\temp.exe\"")
                     .arg(filePath).arg(QFileInfo(filePath).path());
    runCmd = QString("%1\\temp.exe").arg(QFileInfo(filePath).path());


    combinedCmd = compileCmd + " && " + runCmd;

    // 调用 VSCode 打开文件并执行命令
    QProcess *process = new QProcess(this);

    process->start("cmd.exe", QStringList() << "/c" << QString("code \"%1\" && %2").arg(filePath).arg(combinedCmd));

    // 错误处理
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    QMessageBox::warning(this, "执行结果", "汇编程序运行失败，退出代码: " + QString::number(exitCode));
                }
            });
}


