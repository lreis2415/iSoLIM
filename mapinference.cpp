#include "mapinference.h"
#include "ui_mapinference.h"

//#define EXPERIMENT

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
    outputAutoFill = true;
    if(proj==nullptr){
        QMessageBox alertMsg;
        alertMsg.setText("Please create a project first!");
        alertMsg.exec();
        alertMsg.show();
        QTimer::singleShot(0, this, SLOT(close()));;
    } else {
        ui->setupUi(this);
        init = true;
        ui->membership_label->setVisible(false);
        ui->membershipFolder_btn->setVisible(false);
        ui->membershipFolder_lineEdit->setVisible(false);
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
                                                    project->workingDir,
                                                    tr("Output soil file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    if(soilFile.isEmpty()) return;
    else workingDir=QFileInfo(soilFile).absoluteDir().absolutePath();
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
    string membershipFolder = "";
    if(isCategorical && ui->membershipMaps_checkBox->isChecked()){
        QString folder = ui->membershipFolder_lineEdit->text();
        QDir membershipDir(folder);
        if(!membershipDir.exists()||!membershipDir.isEmpty()){
            QMessageBox warn;
            warn.setText("Please assign a valid and empty folder for membership maps.");
            warn.exec();
            return;
        }
        membershipFolder = folder.toStdString();
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
    QFile outsoil_img((outSoil+".png").c_str());
    if(outsoil_img.exists()) outsoil_img.remove();
    QFile outuncer_img((outUncer+".png").c_str());
    if(outuncer_img.exists()) outuncer_img.remove();
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(TRUE);
    ui->cancel_btn->setEnabled(false);
    ui->Inference_OK_btn->setEnabled(false);
    vector<solim::Prototype> *selectedPrototypes = new vector<solim::Prototype>;
    for(size_t i = 0;i<project->prototypes.size();i++){
        if(ui->prototypeBaseName_lineEdit->text().split(';').contains(project->prototypes[i].prototypeBaseName.c_str())){
            solim::Prototype tmp = project->prototypes[i];
            if(tmp.envConditionSize != envFileNames.size()){
                QStringList ruleLayernames;
                for(int i = 0; i<tmp.envConditionSize;i++)
                    ruleLayernames.push_back(tmp.envConditions[i].covariateName.c_str());
                if(ruleLayernames.empty()) continue;
                for(int i = 0; i<envFileNames.size();i++){
                    if(ruleLayernames.contains(layernames[i].c_str())) continue;
                    tmp.addConditions(solim::Curve(layernames[i],solim::getDatatypeFromString(datatypes[i]),true));
                }
            }
            selectedPrototypes->push_back(tmp);
        }
    }
    if(membershipFolder!=""){
        // adjust ramefficient to save memory for writing membership maps
        ramEfficient = ramEfficient*envFileNames.size()/(envFileNames.size()+selectedPrototypes->size());
    }
#ifdef EXPERIMENT
    QTime start = QTime::currentTime();
#endif
    solim::EnvDataset *eds = new solim::EnvDataset(envFileNames,datatypes,layernames,ramEfficient);
    cout<<"Block size: "<<eds->Layers[0]->BlockSize<<endl;
    cout<<"raster size: "<<eds->XSize<<" "<<eds->LayerRef->getYSize()<<endl;
    // update filename
    for(size_t i = 0; i<eds->Layers.size(); i++){
        for(size_t j = 0; j<project->filenames.size(); j++){
            if(eds->Layers.at(i)->LayerName==project->layernames[j]){
                if(eds->Layers.at(i)->baseRef->getFilename()!=project->filenames[j])
                    project->filenames[j] = eds->Layers.at(i)->baseRef->getFilename();
            }
        }
    }
#ifdef EXPERIMENT
    QTime start1 = QTime::currentTime();
#endif
    solim::Inference *infer = new solim::Inference(eds,selectedPrototypes,threshold,outSoil,outUncer);
    if(isCategorical == false){
        try{
            infer->Mapping(targetName,ui->progressBar);
            //solim::Inference::inferMap(eds, selectedPrototypes, targetName, threshold, outSoil, outUncer,ui->progressBar);
        } catch (invalid_argument e) {
            QMessageBox warn;
            warn.setText("Prototype rules inconsistent with covariates used for inference, please check again!");
            return;
        }
    } else {
        try{
            infer->MappingCategorical(targetName,membershipFolder,ui->progressBar);
            //solim::Inference::inferCategoricalMap(eds, selectedPrototypes, targetName, threshold, outSoil, outUncer,membershipFolder,ui->progressBar);
        } catch (invalid_argument e) {
            QMessageBox warn;
            warn.setText("Prototype rules inconsistent with covariates used for inference, please check again!");
            return;
        }
        if(membershipFolder!=""){
            QDir membershipDir(membershipFolder.c_str());
            QFileInfoList membershipMaps = membershipDir.entryInfoList();
            for(QFileInfo map:membershipMaps){
                if(!map.isDir()) project->addResult(map.absoluteFilePath().toStdString());
            }
        }
    }
#ifdef EXPERIMENT
    QTime end = QTime::currentTime();
    cout<<"Start:"<<start.minute()<<":"<<start.second()<<endl;
    cout<<"end:"<<end.minute()<<":"<<end.second()<<endl;
    cout<<"Elasped time: "<<(start.msecsTo(end)/1000.0)<<"s"<<endl;
    cout<<"compute time:"<<infer->compute_time<<"s"<<endl;
    QMessageBox msg;
    msg.setText(QString::number(eds->LayerRef->getBlockSize())+"blocks; Elasped time:"+QString::number(start.msecsTo(end)/1000.0)+" compute time: "+QString::number(infer->compute_time));
    msg.exec();

#endif
    project->addResult(outSoil,infer->outSoilMap->getDataMax(), infer->outSoilMap->getDataMin());
    project->addResult(outUncer,1,0);
    project->currentResultName = outSoil;
    this->close();
}

void mapInference::on_cancel_btn_clicked()
{
    this->close();
}

void mapInference::on_editPrototypeBase_btn_clicked()
{
    QStringList names;
    for(size_t i = 0;i<project->prototypeBaseNames.size();i++)
        names.append(project->prototypeBaseNames[i].c_str());
    itemSelectionWindow editDialog(itemSelectionWindow::PROTOTYPEBASESELECTION, names,ui->prototypeBaseName_lineEdit->text(),this);
    editDialog.exec();
    init = false;
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

void mapInference::on_InferedProperty_comboBox_currentTextChanged(const QString &arg1)
{
    QStringList basenames = ui->prototypeBaseName_lineEdit->text().split(';',Qt::SkipEmptyParts);
    for(int i = 0; i<project->prototypes.size();i++){
        if(project->prototypes[i].prototypeBaseName==basenames.at(0).toStdString()){
            for(int j = 0;j<project->prototypes[i].properties.size();j++){
                if(project->prototypes[i].properties[j].propertyName.c_str()==arg1){
                    if(project->prototypes[i].properties[j].soilPropertyType==solim::CATEGORICAL){
                        isCategorical = true;
                        ui->membershipMaps_checkBox->setVisible(true);
                    } else {
                        isCategorical = false;
                        ui->membershipMaps_checkBox->setVisible(false);
                        ui->membershipFolder_btn->setVisible(false);
                        ui->membership_label->setVisible(false);
                        ui->membershipFolder_lineEdit->setVisible(false);
                    }
                }
            }
            break;
        }
    }
    if(outputAutoFill){
        QString dir = project->workingDir;
        QString ext = ".tif";
        if(project->filenames.size()>0)
            if(QString(project->filenames[0].c_str()).endsWith(".3dr"))
                ext = ".3dr";
        if(dir.indexOf("/")>-1){
            QString soilFile = dir+"/"+arg1+ext;
            QFileInfo soilFileInfo(soilFile);
            int i = 1;
            while(soilFileInfo.exists()){
                soilFile = dir+"/"+arg1+"("+QString::number(i)+")"+ext;
                soilFileInfo.setFile(soilFile);
                i++;
            }
            ui->OutputSoilFile_lineEdit->setText(soilFile);
            QString uncerFile = dir+"/"+arg1+"_uncer"+ext;
            QFileInfo uncerFileInfo(uncerFile);
            i = 1;
            while(uncerFileInfo.exists()){
                uncerFile = dir+"/"+arg1+"_uncer("+QString::number(i)+")"+ext;
                uncerFileInfo.setFile(uncerFile);
                i++;
            }
            ui->OutputUncerFile_lineEdit->setText(uncerFile);
        }
        if(workingDir.indexOf("\\")>-1){
            QString soilFile = dir+"\\"+arg1+ext;
            QFileInfo soilFileInfo(soilFile);
            int i = 1;
            while(soilFileInfo.exists()){
                soilFile = dir+"\\"+arg1+"("+QString::number(i)+")"+ext;
                soilFileInfo.setFile(soilFile);
                i++;
            }
            ui->OutputSoilFile_lineEdit->setText(soilFile);
            QString uncerFile = dir+"\\"+arg1+"_uncer"+ext;
            QFileInfo uncerFileInfo(uncerFile);
            i = 1;
            while(uncerFileInfo.exists()){
                uncerFile = dir+"\\"+arg1+"_uncer("+QString::number(i)+")"+ext;
                uncerFileInfo.setFile(uncerFile);
                i++;
            }
            ui->OutputUncerFile_lineEdit->setText(uncerFile);
        }
    }
}

void mapInference::on_OutputSoilFile_lineEdit_textEdited(const QString &arg1)
{
    outputAutoFill = false;
}

void mapInference::on_OutputUncerFile_lineEdit_textEdited(const QString &arg1)
{
    outputAutoFill = false;
}

void mapInference::on_membershipMaps_checkBox_toggled(bool checked)
{
    if(checked){
        ui->membershipFolder_btn->setVisible(true);
        ui->membership_label->setVisible(true);
        ui->membershipFolder_lineEdit->setVisible(true);
    } else {
        ui->membershipFolder_btn->setVisible(false);
        ui->membership_label->setVisible(false);
        ui->membershipFolder_lineEdit->setVisible(false);
    }
}

void mapInference::on_membershipFolder_btn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                      tr("Open Directory for Membership Maps"),
                                      project->workingDir);
    ui->membershipFolder_lineEdit->setText(dir);
    if(!dir.isEmpty())    project->workingDir = dir;
}

void mapInference::on_prototypeBaseName_lineEdit_textChanged(const QString &arg1)
{
    QStringList basenames = ui->prototypeBaseName_lineEdit->text().split(';',Qt::SkipEmptyParts);
    if(basenames.empty()) return;
    QStringList propertyList;
    for(int k = 0;k < basenames.size();k++){
        QStringList propertyList_tmp;
        for(int i = 0; i<project->prototypes.size();i++){
            if(project->prototypes[i].prototypeBaseName==basenames.at(k).toStdString()){
                for(int j = 0;j<project->prototypes[i].properties.size();j++){
                    propertyList_tmp.push_back(project->prototypes[i].properties[j].propertyName.c_str());
                }
                break;
            }
        }
        if(k==0)
            propertyList = propertyList_tmp;
        else {
            QStringList propertyList_common;
            for(int m = 0; m<propertyList.size();m++){
                for(int n =0; n<propertyList_tmp.size();n++){
                    if(propertyList.at(m)==propertyList_tmp.at(n)){
                        propertyList_common.push_back(propertyList.at(m));
                    }
                }
            }
            propertyList = propertyList_common;
        }
    }
    if (propertyList.isEmpty()) {
        if(!init) {
            if(basenames.size()==1)
                warn("The selected prototype base has no property to be inferred. Please re-select prototype base used for inference.");
            if(basenames.size()>1)
                warn("The selected prototype bases have no common property to be inferred. Please re-select prototype base used for inference.");
        }
        ui->prototypeBaseName_lineEdit->clear();
        ui->Inference_OK_btn->setEnabled(false);
        return;
    }
    ui->InferedProperty_comboBox->clear();
    ui->InferedProperty_comboBox->addItems(propertyList);
    ui->InferedProperty_comboBox->setCurrentIndex(0);
    ui->Inference_OK_btn->setEnabled(true);
}
