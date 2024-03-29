#ifndef ADDGISDATADIALOG_H
#define ADDGISDATADIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>
#include <project.h>

namespace Ui {
class SimpleDialog;
}

class SimpleDialog : public QDialog
{
    Q_OBJECT

public:
    enum simpleDialogMode{ADDGISDATA,ADDCOVARIATE,ADDPROTOTYPEBASE,EDITSTUDYAREA,MODIFYGISDATA,RESAMPLE,RESETRANGE,CHANGEBASENAME,MODIFYLAYERNAME};
    explicit SimpleDialog(simpleDialogMode mode, SoLIMProject *proj, QWidget *parent = nullptr);
    ~SimpleDialog();
    QString filename;
    QString covariate;
    std::string datatype;
    QString lineEdit1;
    QString lineEdit2;
    QString lineEdit3;
    bool nextFlag;
    QString workingDir;
    SoLIMProject *project;

private slots:
    void on_lineEdit_1_textChanged(const QString &arg1);

    void on_btn_1_clicked();

    void on_cancel_btn_clicked();

    void on_ok_btn_clicked();

    void on_next_btn_clicked();

    void on_btn_2_clicked();

    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
    };

    void on_btn_3_clicked();

private:
    Ui::SimpleDialog *ui;
    int mode;
};

#endif // ADDGISDATADIALOG_H
