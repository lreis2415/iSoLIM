#ifndef CHANGECOVNAME_H
#define CHANGECOVNAME_H

#include <QDialog>
#include <QLineEdit>
#include "solim_lib/Prototype.h"

namespace Ui {
class changeCovName;
}

class changeCovName : public QDialog
{
    Q_OBJECT

public:
    explicit changeCovName(solim::Prototype *proto, QWidget *parent = nullptr);
    ~changeCovName();
    solim::Prototype *proto;
    bool isChanged = false;

private slots:
    void on_cancel_btn_clicked();

    void on_ok_btn_clicked();

private:
    Ui::changeCovName *ui;
};

#endif // CHANGECOVNAME_H
