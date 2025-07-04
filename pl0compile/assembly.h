#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <QWidget>

namespace Ui {
class assembly;
}

class assembly : public QWidget
{
    Q_OBJECT

public:
    explicit assembly(QWidget *parent = nullptr);
    QString saveCodeToTempFile(const QString &code, const QString &fileExtension);
    ~assembly();

private slots:

    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();


    void on_pushButton_6_clicked();

private:
    Ui::assembly *ui;
};

#endif // ASSEMBLY_H
