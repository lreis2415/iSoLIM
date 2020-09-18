#include "addgisdatadialog.h"
#include "ui_addgisdatadialog.h"

AddGisDataDialog::AddGisDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddGisDataDialog)
{
    ui->setupUi(this);
    ui->ok_btn->setEnabled(true);
}

AddGisDataDialog::~AddGisDataDialog()
{
    delete ui;
}

void AddGisDataDialog::on_filename_lineEdit_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty())
        ui->ok_btn->setEnabled(true);
}

void AddGisDataDialog::on_browse_btn_clicked()
{
    QString qfilename = QFileDialog::getOpenFileName(this,
                                                    tr("Open environmental covariate file"),
                                                    "./",
                                                    tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    ui->filename_lineEdit->setText(qfilename);
    filename = qfilename;
    if(!filename.isEmpty()){
        std::size_t first = filename.toStdString().find_last_of('/');
        if (first==std::string::npos){
            first = filename.toStdString().find_last_of('\\');
        }
        std::size_t end = filename.toStdString().find_last_of('.');
        covariate = filename.toStdString().substr(first+1,end-first-1).c_str();
    }
    ui->covariate_lineEdit->setText(covariate);
}

void AddGisDataDialog::on_cancel_btn_clicked()
{
    close();
}

void AddGisDataDialog::on_ok_btn_clicked()
{
    filename = ui->filename_lineEdit->text();
    covariate=ui->covariate_lineEdit->text();
    if(ui->checkBox->isChecked()) datatype="CATEGORICAL";
    else datatype="CONTINUOUS";
    close();
}
