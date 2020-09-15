#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);
    ui->studyArea_lineEdit->setVisible(false);
    ui->studyArea_label->setVisible(false);
    ui->projPath_lineEdit->setText(QDir::currentPath());
    ui->ok_btn->setEnabled(false);
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

void NewProjectDialog::on_pushButton_clicked()
{
    QString path=QFileDialog::getExistingDirectory(this,
                                                   tr("Choose project directory"),
                                                   "./");
    ui->projPath_lineEdit->setText(path);
}

void NewProjectDialog::on_cancel_btn_clicked()
{
    projectName="";
    studyArea="";
    close();
}

void NewProjectDialog::on_ok_btn_clicked()
{
    projectName=ui->projName_lineEdit->text();
    projectFilename=QDir(ui->projPath_lineEdit->text()).filePath(projectName)+".slp";
    studyArea=ui->studyArea_lineEdit->text();
    close();
}



void NewProjectDialog::on_projName_lineEdit_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty()&&!ui->projPath_lineEdit->text().isEmpty())
        ui->ok_btn->setEnabled(true);
}

void NewProjectDialog::on_projPath_lineEdit_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty()&&!ui->projName_lineEdit->text().isEmpty())
        ui->ok_btn->setEnabled(true);
}

void NewProjectDialog::on_studyArea_radioButton_toggled(bool checked)
{
    if(checked){
        ui->studyArea_lineEdit->setVisible(true);
        ui->studyArea_label->setVisible(true);
    } else {
        ui->studyArea_lineEdit->setVisible(false);
        ui->studyArea_label->setVisible(false);
    }
}
