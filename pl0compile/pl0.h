#ifndef PL__H
#define PL__H

#include <QWidget>

namespace Ui {
class pl0;
}

class pl0 : public QWidget
{
    Q_OBJECT

public:
    explicit pl0(QWidget *parent = nullptr);
    ~pl0();

private:
    Ui::pl0 *ui;
};

#endif // PL__H
