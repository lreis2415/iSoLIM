#ifndef INTELLIGENTINFER_H
#define INTELLIGENTINFER_H

#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QTimer>
#include <QCheckBox>
#include "project.h"
#include "itemselectionwindow.h"
#include "simpledialog.h"
#include "solim_lib/inference.h"
#include "QTime"

namespace Ui {
class intelligentInfer;
}

class intelligentInfer : public QDialog
{
    Q_OBJECT

public:
    explicit intelligentInfer(SoLIMProject *proj, QWidget *parent = nullptr);
    QString workingDir;
    ~intelligentInfer();
signals:
    void inferred();
private slots:

    void on_Inference_OK_btn_clicked();

    void on_cancel_btn_clicked();

    void tableItemClicked(int,int);

    void on_SampleFile_btn_clicked();

    void on_AddCov_btn_clicked();

    void on_OutputFile_btn_clicked();

private:
    Ui::intelligentInfer *ui;
    SoLIMProject *project;
    QStringList categoricalProps;
    bool init;
    bool outputAutoFill;
    bool isCategorical;
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }
};

#endif // INTELLIGENTINFER_H
