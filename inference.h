#ifndef INFERENCE_H
#define INFERENCE_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QCheckBox>
#include "project.h"
#include "editprototypebases.h"
#include "simpledialog.h"

namespace Ui {
class inference;
}

class inference : public QDialog
{
    Q_OBJECT

public:
    explicit inference(SoLIMProject *proj, QWidget *parent = nullptr);
    QString workingDir;
    ~inference();
signals:
    void inferred();
private slots:
    void on_SoilFileCreate_btn_clicked();

    void on_Inference_OK_btn_clicked();

    void on_cancel_btn_clicked();

    void on_editPrototypeBase_btn_clicked();

    void tableItemClicked(int,int);

private:
    Ui::inference *ui;
    SoLIMProject *project;
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }
};

#endif // INFERENCE_H
