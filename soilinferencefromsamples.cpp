#include "soilinferencefromsamples.h"
#include "ui_soilinferencefromsamples.h"
#include "QFileDialog"
#include "QDebug"

soilInferenceFromSamples::soilInferenceFromSamples(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::soilInferenceFromSamples)
{
    ui->setupUi(this);
    ui->RAMEfficient_medium_rbtn->setChecked(TRUE);
    ui->progressBar->setVisible(FALSE);
    ui->CovariateFiles_tableWidget->setColumnCount(3);
    ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("File Name"));
    ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Datatype"));
    ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("Layer Name"));
    ui->Threshold_lineEdit->setText("0.0");
}

soilInferenceFromSamples::~soilInferenceFromSamples()
{
    delete ui;
}

void soilInferenceFromSamples::on_SampleFileRead_btn_clicked()
{
    QString samplesFileName = QFileDialog::getOpenFileName(this,
                                                           tr("Open samples file"),
                                                           "./",
                                                           tr("Sample files(*.csv *.txt)"));
    ui->SampleFileName_lineEdit->setText(samplesFileName);
    QFile sampleFile(samplesFileName);
    if(!sampleFile.open(QIODevice::ReadOnly))
         qDebug()<<"OPEN FILE FAILED";
    QTextStream *out = new QTextStream(&sampleFile);//文本流
    QStringList tempOption = out->readAll().split("\n");//每行以\n区分
    QStringList columnNames = tempOption.at(0).split(",");
    ui->InferedProperty_comboBox->addItems(columnNames);
    ui->ID_comboBox->addItems(columnNames);
}

void soilInferenceFromSamples::on_CovariateFileRead_btn_clicked()
{
    QString covariateName = QFileDialog::getOpenFileName(this,
                                                           tr("Open environmental covariate file"),
                                                           "./",
                                                           tr("Sample files(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    ui->CovariateFiles_tableWidget->insertRow(ui->CovariateFiles_tableWidget->rowCount());
    ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            0,
                                            new QTableWidgetItem(covariateName));
    QComboBox *datatype = new QComboBox();
    datatype->addItem("CONTINUOUS");
    datatype->addItem("CATEGORICAL");
    ui->CovariateFiles_tableWidget->setCellWidget(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            1,
                                            datatype);
    std::size_t first = covariateName.toStdString().find_last_of('/');
    if (first==std::string::npos){
        first = covariateName.toStdString().find_last_of('\\');
    }
    std::size_t end = covariateName.toStdString().find_last_of('.');
    string layerName = covariateName.toStdString().substr(first+1,end-first-1);
    ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            2,
                                            new QTableWidgetItem(QString::fromStdString(layerName)));
}

void soilInferenceFromSamples::on_CovariateFileDelete_btn_clicked()
{
    ui->CovariateFiles_listWidget->takeItem(ui->CovariateFiles_listWidget->currentRow());
    ui->CovariateFiles_tableWidget->removeRow(ui->CovariateFiles_tableWidget->currentRow());
}
void soilInferenceFromSamples::on_InferFromSmaples_OK_btn_clicked()
{
    string sampleFile = ui->SampleFileName_lineEdit->text().toStdString();
    vector<string> envFileNames;
    vector<string> datatypes;
    vector<string> layernames;
    for(int i = 0; i<ui->CovariateFiles_tableWidget->rowCount();++i){
        envFileNames.push_back(ui->CovariateFiles_tableWidget->item(i,0)->text().toStdString());
        QComboBox *type = (QComboBox*) ui->CovariateFiles_tableWidget->cellWidget(i,1);
        datatypes.push_back(type->currentText().toStdString());
        layernames.push_back(ui->CovariateFiles_tableWidget->item(i,2)->text().toStdString());
    }
    double ramEfficient;
    if(ui->RAMEfficient_low_rbtn->isChecked()){
        ramEfficient=0.25;
    } else if(ui->RAMEfficient_high_rbtn->isChecked()){
        ramEfficient=0.75;
    } else
        ramEfficient=0.5;
    double threshold=atof(ui->Threshold_lineEdit->text().toStdString().c_str());
    string targetName = ui->InferedProperty_comboBox->currentText().toStdString();
    string idName = ui->ID_comboBox->currentText().toStdString();
    string outSoil = ui->OutputSoilFile_lineEdit->text().toStdString();
    string outUncer=ui->OutputUncerFile_lineEdit->text().toStdString();
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(TRUE);
    ui->InferFromSmaples_OK_btn->setEnabled(false);
    solim::Inference::iPSMInferSoil(envFileNames,datatypes,layernames,threshold,sampleFile,
                                    targetName,idName,outSoil,outUncer,ramEfficient,ui->progressBar);
}

void soilInferenceFromSamples::on_buttonBox_accepted(){}

void soilInferenceFromSamples::on_SoilFileCreate_btn_clicked()
{
    QString soilFile = QFileDialog::getSaveFileName(this,
                                                    tr("Save output soil file"),
                                                    "./",
                                                    tr("Sample files(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    ui->OutputSoilFile_lineEdit->setText(soilFile);
}

void soilInferenceFromSamples::on_UncerFileCreate_btn_clicked()
{
    QString uncerFile = QFileDialog::getSaveFileName(this,
                                                 tr("Save output uncertainty file"),
                                                 "./",
                                                 tr("Sample files(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    ui->OutputUncerFile_lineEdit->setText(uncerFile);

}
