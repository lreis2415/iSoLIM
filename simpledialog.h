#ifndef ADDGISDATADIALOG_H
#define ADDGISDATADIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
class SimpleDialog;
}

class SimpleDialog : public QDialog
{
    Q_OBJECT

public:
    enum simpleDialogMode{ADDGISDATA,ADDCOVARIATE,ADDPROTOTYPEBASE,ADDPROTOTYPE};
    explicit SimpleDialog(int mode, QWidget *parent = nullptr);
    ~SimpleDialog();
    QString filename;
    QString covariate;
    std::string datatype;
    QString lineEdit2;
    bool nextFlag;

private slots:
    void on_lineEdit_1_textChanged(const QString &arg1);

    void on_browse_btn_clicked();

    void on_cancel_btn_clicked();

    void on_ok_btn_clicked();

    void on_next_btn_clicked();

private:
    Ui::SimpleDialog *ui;
    int mode;
};

#endif // ADDGISDATADIALOG_H
