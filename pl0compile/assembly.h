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
    ~assembly();

private:
    Ui::assembly *ui;
};

#endif // ASSEMBLY_H
