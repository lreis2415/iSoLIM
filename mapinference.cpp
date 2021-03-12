#include "mapinference.h"
#include "ui_mapinference.h"

mapInference::mapInference(SoLIMProject *proj, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mapInference),
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
            }
            ui->prototypeBaseName_lineEdit->setText(project->prototypeBaseNames[0].c_str());
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
            connect(ui->CovariateFiles_tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(tableItemClicked(int,int)));
            QStringList propertyList;
            for(int i = 0; i<proj->prototypes.size();i++){
                if(proj->prototypes[i].prototypeBaseName==proj->prototypeBaseNames[0]){
                    for(int j = 0;j<proj->prototypes[i].properties.size();j++){
                        propertyList.push_back(proj->prototypes[i].properties[j].propertyName.c_str());
                    }
                    break;
                }
            }
            ui->InferedProperty_comboBox->addItems(propertyList);
        }
    }
}

mapInference::~mapInference()
{
    delete ui;
}

void mapInference::on_SoilFileCreate_btn_clicked()
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

void mapInference::on_Inference_OK_btn_clicked()
{
    if(ui->prototypeBaseName_lineEdit->text().isEmpty()){
        QMessageBox warn;
        warn.setText("Please specify the prototype base(s) used for inference.");
        warn.exec();
        return;
    }
    if(ui->OutputSoilFile_lineEdit->text().isEmpty()||ui->OutputUncerFile_lineEdit->text().isEmpty()){
        warn("Please fill in the output filename!");
        return;
    }
    vector<string> envFileNames;
    vector<string> datatypes;
    vector<string> layernames;
    for(int i = 0; i<ui->CovariateFiles_tableWidget->rowCount();++i){
        QCheckBox *selected = (QCheckBox*) ui->CovariateFiles_tableWidget->cellWidget(i,3);
        if(!selected->isChecked())
            continue;
        QString filename = ui->CovariateFiles_tableWidget->item(i,0)->text();
        QString layername = ui->CovariateFiles_tableWidget->item(i,1)->text();
        QFileInfo fileinfo(filename);
        if(!fileinfo.exists()){
            QMessageBox warn;
            warn.setText("Filename for layer "+layername+" is invalid!");
            warn.exec();
            return;
        }
        envFileNames.push_back(filename.toStdString());
        QCheckBox *type = (QCheckBox*) ui->CovariateFiles_tableWidget->cellWidget(i,2);
        if(type->isChecked())
            datatypes.push_back("CATEGORICAL");
        else datatypes.push_back("CONTINUOUS");
        layernames.push_back(layername.toStdString());
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
    ui->cancel_btn->setEnabled(false);
    ui->Inference_OK_btn->setEnabled(false);
    solim::EnvDataset *eds = new solim::EnvDataset(envFileNames,datatypes,layernames,ramEfficient);
    vector<solim::Prototype> *prototypes = new vector<solim::Prototype>;
    for(size_t i = 0;i<project->prototypes.size();i++){
        //if(project->prototypes[i].prototypeBaseName == ui->prototypeBaseName_lineEdit->text().toStdString())
        if(ui->prototypeBaseName_lineEdit->text().split(';').contains(project->prototypes[i].prototypeBaseName.c_str()))
            prototypes->push_back(project->prototypes[i]);
    }
    solim::Inference::inferMap(eds, &(project->prototypes), targetName, threshold, outSoil, outUncer,ui->progressBar);

    project->results.push_back(outSoil);
    project->results.push_back(outUncer);;
    this->close();
}

void mapInference::on_cancel_btn_clicked()
{
    this->close();
}

void mapInference::on_editPrototypeBase_btn_clicked()
{
    //todo
    QStringList names;
    for(size_t i = 0;i<project->prototypeBaseNames.size();i++)
        names.append(project->prototypeBaseNames[i].c_str());
    editPrototypeBases editDialog(names,ui->prototypeBaseName_lineEdit->text(),this);
    editDialog.exec();
    ui->prototypeBaseName_lineEdit->setText(editDialog.selectedNames);
}

void mapInference::tableItemClicked(int row,int col){
    if(col == 0){
        SimpleDialog addGisData(SimpleDialog::MODIFYGISDATA,project, this);
        addGisData.exec();
        ui->CovariateFiles_tableWidget->setItem(row,
                                                0,
                                                new QTableWidgetItem(addGisData.filename));
    }
}