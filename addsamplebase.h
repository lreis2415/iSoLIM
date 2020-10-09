#ifndef ADDSAMPLEBASE_H
#define ADDSAMPLEBASE_H

#include <QDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "project.h"
#include "solim-lib-forqt.h"
using solim::Prototype;
namespace Ui {
class AddSampleBase;
}

class AddSampleBase : public QDialog
{
    Q_OBJECT

public:
    explicit AddSampleBase(SoLIMProject *proj, QWidget *parent = nullptr);
    ~AddSampleBase();

private slots:
    void on_addCovariate_btn_clicked();

    void on_deleteCovariate_btn_clicked();

    void on_browseSampleFile_btn_clicked();

    //void on_buttonBox_accepted();

    void on_ok_btn_clicked();

private:
    Ui::AddSampleBase *ui;
    SoLIMProject *project;
    //QThread thread;
};

#endif // ADDSAMPLEBASE_H
