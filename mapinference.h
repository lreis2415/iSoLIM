#ifndef MAPINFERENCE_H
#define MAPINFERENCE_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QCheckBox>
#include "project.h"
#include "itemselectionwindow.h"
#include "simpledialog.h"
#include "solim_lib/inference.h"

namespace Ui {
class mapInference;
}

class mapInference : public QDialog
{
    Q_OBJECT

public:
    explicit mapInference(SoLIMProject *proj, QWidget *parent = nullptr);
    QString workingDir;
    ~mapInference();
signals:
    void inferred();
private slots:
    void on_SoilFileCreate_btn_clicked();

    void on_Inference_OK_btn_clicked();

    void on_cancel_btn_clicked();

    void on_editPrototypeBase_btn_clicked();

    void tableItemClicked(int,int);

    void on_InferedProperty_comboBox_currentTextChanged(const QString &arg1);

    void on_OutputSoilFile_lineEdit_textEdited(const QString &arg1);

    void on_OutputUncerFile_lineEdit_textEdited(const QString &arg1);

    void on_membershipMaps_checkBox_toggled(bool checked);

    void on_membershipFolder_btn_clicked();

    void on_prototypeBaseName_lineEdit_textChanged(const QString &arg1);

private:
    Ui::mapInference *ui;
    SoLIMProject *project;
    bool init;
    bool outputAutoFill;
    bool isCategorical;
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }
};

#endif // MAPINFERENCE_H
