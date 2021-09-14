#ifndef ADDPROTOTYPEBASE_H
#define ADDPROTOTYPEBASE_H

#include <QDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "project.h"
#include "simpledialog.h"
using solim::Prototype;
namespace Ui {
class AddPrototypeBase;
}

class AddPrototypeBase : public QDialog
{
    Q_OBJECT

public:
    enum addPrototypeBaseMode{SAMPLE, MAP};
    int addedLayer;
    explicit AddPrototypeBase(addPrototypeBaseMode mode, SoLIMProject *proj, QWidget *parent = nullptr);
    ~AddPrototypeBase();

private slots:
    void on_addCovariate_btn_clicked();

    void on_deleteCovariate_btn_clicked();

    void on_browseSampleFile_btn_clicked();

    void on_ok_btn_clicked();

    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
    };

    void on_btn_hint_clicked();

    void on_covariate_tableWidget_itemSelectionChanged();

private:
    addPrototypeBaseMode mode;
    Ui::AddPrototypeBase *ui;
    SoLIMProject *project;
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }
};

#endif // ADDSAMPLEBASE_H
