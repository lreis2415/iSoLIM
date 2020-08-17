#ifndef INFERENCE_H
#define INFERENCE_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QCheckBox>
#include "project.h"

namespace Ui {
class inference;
}

class inference : public QDialog
{
    Q_OBJECT

public:
    explicit inference(SoLIMProject *proj, QWidget *parent = nullptr);
    ~inference();

private slots:
    void on_SoilFileCreate_btn_clicked();

    void on_Inference_OK_btn_clicked();

private:
    Ui::inference *ui;
    SoLIMProject *project;
};

#endif // INFERENCE_H
