#ifndef ADDGISDATADIALOG_H
#define ADDGISDATADIALOG_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class AddGisDataDialog;
}

class AddGisDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddGisDataDialog(QWidget *parent = nullptr);
    ~AddGisDataDialog();
    QString filename;
    QString covariate;
    std::string datatype;

private slots:
    void on_filename_lineEdit_textChanged(const QString &arg1);

    void on_browse_btn_clicked();

    void on_cancel_btn_clicked();

    void on_ok_btn_clicked();

private:
    Ui::AddGisDataDialog *ui;
};

#endif // ADDGISDATADIALOG_H
