#include "simpledialog.h"
#include "ui_simpledialog.h"

SimpleDialog::SimpleDialog(int mode, QWidget *parent) :
    mode(mode),QDialog(parent),
    ui(new Ui::SimpleDialog)
{
    ui->setupUi(this);
    switch (mode) {
    case ADDGISDATA:
        ui->next_btn->setVisible(false);
        ui->ok_btn->setEnabled(false);
        break;
    case ADDCOVARIATE:
        ui->next_btn->setVisible(false);
        break;
    case ADDPROTOTYPEBASE:
    case ADDPROTOTYPE:
        this->setWindowTitle("Create Prototype Base from Expert");
        ui->label_1->setVisible(false);
        ui->lineEdit_1->setVisible(false);
        ui->browse_btn->setVisible(false);
        ui->label_2->setText("Create Prototype Base");
        ui->checkBox->setVisible(false);
    }
    if(mode==ADDPROTOTYPE){
        setWindowTitle("Create prototype from Expert");
        ui->label_2->setText("Create Prototype");
    }
    filename="";
    covariate="";
    lineEdit2="";
    nextFlag=false;
}

SimpleDialog::~SimpleDialog()
{
    delete ui;
}

void SimpleDialog::on_lineEdit_1_textChanged(const QString &arg1)
{
    if(mode==ADDGISDATA){
        if(!arg1.isEmpty())
            ui->ok_btn->setEnabled(true);
    }
}

void SimpleDialog::on_browse_btn_clicked()
{
    if(mode==ADDGISDATA||mode==ADDCOVARIATE){
        QString qfilename = QFileDialog::getOpenFileName(this,
                                                        tr("Open environmental covariate file"),
                                                        "./",
                                                        tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        ui->lineEdit_1->setText(qfilename);
        filename = qfilename;
        if(!filename.isEmpty()){
            std::size_t first = filename.toStdString().find_last_of('/');
            if (first==std::string::npos){
                first = filename.toStdString().find_last_of('\\');
            }
            std::size_t end = filename.toStdString().find_last_of('.');
            covariate = filename.toStdString().substr(first+1,end-first-1).c_str();
        }
        ui->lineEdit_2->setText(covariate);
    }
}

void SimpleDialog::on_cancel_btn_clicked()
{
    filename.clear();
    covariate.clear();
    lineEdit2.clear();
    close();
}

void SimpleDialog::on_ok_btn_clicked()
{
    if(mode==ADDGISDATA||mode==ADDCOVARIATE){
        filename = ui->lineEdit_1->text();
        covariate=ui->lineEdit_2->text();
        if(ui->checkBox->isChecked()) datatype="CATEGORICAL";
        else datatype="CONTINUOUS";
        QFile f(filename);
        if(mode==ADDGISDATA){
            if(f.exists()){
                close();
            } else {
                QMessageBox warn;
                warn.setText("File does not exist.");
                warn.exec();
            }
        }else {
            if(!f.exists()) filename="";
            close();
        }
    }
    if(mode==ADDPROTOTYPE||mode==ADDPROTOTYPEBASE){
        if(ui->lineEdit_2->text().isEmpty()){
            lineEdit2.clear();
            nextFlag=false;
            return;
        }
        else{
            lineEdit2=ui->lineEdit_2->text();
            close();
        }
    }
}

void SimpleDialog::on_next_btn_clicked(){
    nextFlag=true;
    on_ok_btn_clicked();
}
