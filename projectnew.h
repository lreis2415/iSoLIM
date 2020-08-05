#ifndef PROJECTNEW_H
#define PROJECTNEW_H

#include <QDialog>

namespace Ui {
class ProjectNew;
}

class ProjectNew : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectNew(QWidget *parent = nullptr);
    ~ProjectNew();
    QString projName;
    QString projDirec;

private slots:
    void on_browseProjDirec_btn_clicked();

    void on_buttonBox_accepted();

    void on_projectName_lineEdit_textChanged(const QString &arg1);

    void on_projectDirec_lineEdit_textChanged(const QString &arg1);

private:
    Ui::ProjectNew *ui;
};

#endif // PROJECTNEW_H
