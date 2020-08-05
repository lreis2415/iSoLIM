#include "projectnew.h"
#include "ui_projectnew.h"
#include "QFileDialog"
#include "QMessageBox"
ProjectNew::ProjectNew(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectNew)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ProjectNew::~ProjectNew()
{
    delete ui;
}

void ProjectNew::on_browseProjDirec_btn_clicked()
{
    QString projectDirectory = QFileDialog::getExistingDirectory(this,
                                                        tr("Open working directory"),
                                                        "./");
    ui->projectDirec_lineEdit->setText(projectDirectory);
}

void ProjectNew::on_buttonBox_accepted()
{
    projName = ui->projectName_lineEdit->text();
    projDirec = ui->projectDirec_lineEdit->text();

}


void ProjectNew::on_projectName_lineEdit_textChanged(const QString &arg1)
{
    if(!ui->projectName_lineEdit->text().isEmpty()&&!ui->projectDirec_lineEdit->text().isEmpty())
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void ProjectNew::on_projectDirec_lineEdit_textChanged(const QString &arg1)
{
    if(!ui->projectName_lineEdit->text().isEmpty()&&!ui->projectDirec_lineEdit->text().isEmpty())
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
