#include "validation.h"
#include "ui_validation.h"

Validation::Validation(SoLIMProject *project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Validation)
{
    ui->setupUi(this);
    for(size_t i = 0; i < project->results.size();i++){
        ui->comboBox->addItem(project->results[i].c_str());
    }
    ui->comboBox->addItem("[Select other result file]");
    ui->label_result->setText("");
    ui->label_result->setTextFormat(Qt::RichText);
    ui->label_result->setWordWrap(true);
    proj = project;
}

Validation::~Validation()
{
    delete ui;
}

void Validation::on_btn_browse_clicked()
{
    QString qfilename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Validation Sample File"),
                                                    proj->workingDir,
                                                    tr("Sample file(*.csv *.txt)"));
    ui->lineEdit_sample->setText(qfilename);
}

void Validation::on_comboBox_activated(const QString &arg1)
{
    if(arg1=="[Select other result file]"){
        QString qfilename = QFileDialog::getOpenFileName(this,
                                                        tr("Open Result File"),
                                                        proj->workingDir,
                                                        tr("Result file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        ui->comboBox->insertItem(ui->comboBox->count()-1,qfilename);
        ui->comboBox->setCurrentIndex(ui->comboBox->count()-2);
    }
}

void Validation::on_lineEdit_sample_textChanged(const QString &arg1)
{
    if(QFileInfo(arg1).exists()){
        ifstream file(arg1.toStdString()); // declare file stream:
        if(!file.is_open()){
            QMessageBox warn;
            warn.setText("Cannot open sample file");
            warn.exec();
            ui->lineEdit_sample->clear();
            return;
        }
        string line;
        getline(file, line);
        vector<string> names;
        solim::ParseStr(line, ',', names);
        QStringList columnNames;
        string x="";
        string y ="";
        for(size_t i = 0;i<names.size();i++){
            columnNames.append(names[i].c_str());
            if(names[i]=="x"||names[i]=="X")
                x=names[i];
            if(names[i]=="y"||names[i]=="Y")
                y=names[i];
        }
        ui->comboBox_fieldname->addItems(columnNames);
        ui->comboBox_fieldx->addItems(columnNames);
        ui->comboBox_fieldy->addItems(columnNames);
        if(x!="") ui->comboBox_fieldx->setCurrentText(x.c_str());
        if(y!="") ui->comboBox_fieldy->setCurrentText(y.c_str());
    }
}

void Validation::on_btn_ok_clicked()
{
    if(ui->lineEdit_sample->text().isEmpty()){
        QMessageBox warn;
        warn.setText("Please input validation sample file!");
        warn.exec();
        ui->label_result->clear();
        return;
    }
    string sampleFile = ui->lineEdit_sample->text().toStdString();
    string resultFile = ui->comboBox->currentText().toStdString();
    if(!QFileInfo(sampleFile.c_str()).exists()){
        QMessageBox warn;
        warn.setText("Validation sample file does not exist.");
        warn.exec();
        ui->label_result->clear();
        return;
    }
    if(!QFileInfo(resultFile.c_str()).exists()){
        QMessageBox warn;
        warn.setText("Result file does not exist.");
        warn.exec();
        ui->label_result->clear();
        return;
    }
    string resultDatatype = "CONTINUOUS";
    if(ui->checkBox->isChecked()) resultDatatype = "CATEGORICAL";
    solim::Validate validate(sampleFile, resultFile, resultDatatype,
                             ui->comboBox_fieldx->currentText().toStdString(),
                             ui->comboBox_fieldy->currentText().toStdString(),
                             ui->comboBox_fieldname->currentText().toStdString());
    if(validate.sampleSize==0){
        //warn about no sample
        QMessageBox warn;
        warn.setText("Samples are not outside of result file. Please make sure that samples have the same projection with result file.");
        warn.exec();
        ui->label_result->clear();
        return;
    }
    double rmse = validate.getRMSE();
    double mae = validate.getMAE();
    ui->label_result->setText("<b>RMSE</b>: "+QString::number(rmse)+" ; <b>MAE</b>: "+QString::number(mae));
}
