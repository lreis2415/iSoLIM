#ifndef VALIDATION_H
#define VALIDATION_H

#include <QDialog>
#include <QFileDialog>
#include "project.h"
#include "solim_lib/validate.h"

namespace Ui {
class Validation;
}

class Validation : public QDialog
{
    Q_OBJECT

public:
    explicit Validation(SoLIMProject *project, QWidget *parent = nullptr);
    ~Validation();

private slots:
    void on_btn_browse_clicked();

    void on_comboBox_activated(const QString &arg1);

    void on_lineEdit_sample_textChanged(const QString &arg1);

    void on_btn_ok_clicked();

private:
    Ui::Validation *ui;
    SoLIMProject *proj;
};

#endif // VALIDATION_H
