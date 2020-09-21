#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = nullptr);
    ~NewProjectDialog();
    QString projectName;
    QString projectFilename;
    QString studyArea;

private slots:
    void on_pushButton_clicked();

    void on_cancel_btn_clicked();

    void on_ok_btn_clicked();

    void on_projName_lineEdit_textChanged(const QString &arg1);

    void on_projPath_lineEdit_textChanged(const QString &arg1);

    void on_studyArea_radioButton_toggled(bool checked);

private:
    Ui::NewProjectDialog *ui;
};

#endif // NEWPROJECTDIALOG_H
