#include "inference.h"
#include "ui_inference.h"

inference::inference(SoLIMProject *proj, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inference),
    project(proj)
{
    setWindowFlags(Qt::Window
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint);
    show();
    setWindowIcon(QIcon("./imgs/solim.jpg"));
    if(proj==nullptr){
        QMessageBox alertMsg;
        alertMsg.setText("Please create a project first!");
        alertMsg.exec();
        alertMsg.show();
        QTimer::singleShot(0, this, SLOT(close()));;
    } else {
        ui->setupUi(this);
        ui->progressBar->setVisible(false);
        ui->RAMEfficient_low_rbtn->setChecked(true);
        ui->Threshold_lineEdit->setText("0.0");
        if(project->prototypeBaseNames.size()<1){
            QMessageBox alertMsg;
            alertMsg.setText("Please add prototypes to project first!");
            alertMsg.exec();
            QTimer::singleShot(0, this, SLOT(close()));
        } else{
            if(project->prototypeBaseNames.size()==1){
                ui->editPrototypeBase_btn->setEnabled(false);
                ui->editPrototypeBase_btn->setVisible(false);
                ui->prototypeBaseName_lineEdit->setText(project->prototypeBaseNames[0].c_str());
            }
            ui->RAMEfficient_low_rbtn->setChecked(true);
            ui->Threshold_lineEdit->setText("0.0");
            ui->CovariateFiles_tableWidget->setColumnCount(4);
            ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Filename"));
            ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Covariate"));
            ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("Categorical?"));
            ui->CovariateFiles_tableWidget->setHorizontalHeaderItem(3,new QTableWidgetItem("Selected?"));
            for(int i = 0; i<proj->filenames.size();i++){
                ui->CovariateFiles_tableWidget->setRowCount(ui->CovariateFiles_tableWidget->rowCount()+1);
                ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                                        0,
                                                        new QTableWidgetItem(proj->filenames[i].c_str()));
                ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                                        1,
                                                        new QTableWidgetItem(proj->layernames[i].c_str()));
                QCheckBox *categoriacl_cb = new QCheckBox();
                if(proj->layertypes[i]=="CATEGORICAL")
                    categoriacl_cb->setChecked(true);
                else
                    categoriacl_cb->setChecked(false);
                ui->CovariateFiles_tableWidget->setCellWidget(ui->CovariateFiles_tableWidget->rowCount()-1,
                                                        2,
                                                        categoriacl_cb);
                QCheckBox *selected_cb = new QCheckBox();
                selected_cb->setChecked(true);
                ui->CovariateFiles_tableWidget->setCellWidget(ui->CovariateFiles_tableWidget->rowCount()-1,
                                                              3,
                                                              selected_cb);
            }

            QStringList propertyList;
            for(int i = 0; i<proj->propertyNames.size();i++){
                propertyList.push_back(proj->propertyNames[i].c_str());
            }
            ui->InferedProperty_comboBox->addItems(propertyList);
        }
    }
}

inference::~inference()
{
    delete ui;
}

void inference::on_SoilFileCreate_btn_clicked()
{
    QString soilFile = QFileDialog::getSaveFileName(this,
                                                    tr("Save output soil file"),
                                                    "./",
                                                    tr("Output soil file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    if(soilFile.isEmpty()) return;
    ui->OutputSoilFile_lineEdit->setText(soilFile);
    string uncerFile = soilFile.toStdString();
    std::size_t end = uncerFile.find_last_of('.');
    uncerFile = uncerFile.substr(0,end)+"_uncer"+uncerFile.substr(end);
    ui->OutputUncerFile_lineEdit->setText(uncerFile.c_str());
}

void inference::on_Inference_OK_btn_clicked()
{
    ui->cancel_btn->setEnabled(false);
    if(ui->OutputSoilFile_lineEdit->text().isEmpty()||ui->OutputUncerFile_lineEdit->text().isEmpty()){
        QMessageBox warning;
        warning.setText("Please fill in the output filename!");
        warning.setStandardButtons(QMessageBox::Ok);
        warning.exec();
        return;
    }
    vector<string> envFileNames;
    vector<string> datatypes;
    vector<string> layernames;
    for(int i = 0; i<ui->CovariateFiles_tableWidget->rowCount();++i){
        QCheckBox *selected = (QCheckBox*) ui->CovariateFiles_tableWidget->cellWidget(i,3);
        if(!selected->isChecked())
            continue;
        envFileNames.push_back(ui->CovariateFiles_tableWidget->item(i,0)->text().toStdString());
        QCheckBox *type = (QCheckBox*) ui->CovariateFiles_tableWidget->cellWidget(i,2);
        if(type->isChecked())
            datatypes.push_back("CATEGORICAL");
        else datatypes.push_back("CONTINUOUS");
        layernames.push_back(ui->CovariateFiles_tableWidget->item(i,1)->text().toStdString());
    }
    double ramEfficient;
    if(ui->RAMEfficient_low_rbtn->isChecked()){
        ramEfficient=0.25;
    } else if(ui->RAMEfficient_high_rbtn->isChecked()){
        ramEfficient=0.75;
        QMessageBox highMsg;
        highMsg.setText("High RAM Efficiency is checked. Please do not open large software while inference is running if input layers are large!");
        highMsg.exec();
        highMsg.show();
    } else
        ramEfficient=0.5;
    double threshold=atof(ui->Threshold_lineEdit->text().toStdString().c_str());
    string targetName = ui->InferedProperty_comboBox->currentText().toStdString();
    string outSoil = ui->OutputSoilFile_lineEdit->text().toStdString();
    string outUncer=ui->OutputUncerFile_lineEdit->text().toStdString();
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(TRUE);
    ui->Inference_OK_btn->setEnabled(false);
    solim::EnvDataset *eds = new solim::EnvDataset(envFileNames,datatypes,layernames,ramEfficient);
    vector<solim::Prototype> *prototypes = new vector<solim::Prototype>;
    for(int i = 0;i<project->prototypes.size();i++){
        if(project->prototypes[i].prototypeBaseName == ui->prototypeBaseName_lineEdit->text().toStdString())
            prototypes->push_back(project->prototypes[i]);
    }
    solim::Inference::inferMap(eds, &(project->prototypes), targetName, threshold, outSoil, outUncer,ui->progressBar);

    project->results.push_back(outSoil);
    project->results.push_back(outUncer);
    this->close();
}

void inference::on_cancel_btn_clicked()
{
    this->close();
}
