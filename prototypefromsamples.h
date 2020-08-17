#ifndef PROTOTYPEFROMSAMPLES_H
#define PROTOTYPEFROMSAMPLES_H

#include <QDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include "project.h"
#include "solim-lib-forqt.h"
using solim::Prototype;

namespace Ui {
class prototypeFromSamples;
}

class prototypeFromSamples : public QDialog
{
    Q_OBJECT

public:
    explicit prototypeFromSamples(SoLIMProject *proj, QWidget *parent = nullptr);
    ~prototypeFromSamples();

private slots:
    void on_addCovariate_btn_clicked();

    void on_deleteCovariate_btn_clicked();

    void on_browseSampleFile_btn_clicked();

    //void on_buttonBox_accepted();

    void on_ok_btn_clicked();

private:
    Ui::prototypeFromSamples *ui;
    SoLIMProject *project;
    void updateLabel();
};

#endif // PROTOTYPEFROMSAMPLES_H
