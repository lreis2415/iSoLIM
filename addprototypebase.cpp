#include "addprototypebase.h"
#include "ui_addprototypebase.h"

AddPrototypeBase::AddPrototypeBase(addPrototypeBaseMode mode,SoLIMProject *proj,QWidget *parent) :
    mode(mode), QDialog(parent),
    ui(new Ui::AddPrototypeBase)
{
    ui->setupUi(this);
    project = proj;
    ui->progressBar->setVisible(false);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(2);
    ui->covariate_tableWidget->clear();
    ui->covariate_tableWidget->setColumnCount(3);
    ui->covariate_tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Filename"));
    ui->covariate_tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("Covariate"));
    ui->covariate_tableWidget->setHorizontalHeaderItem(2,new QTableWidgetItem("Categorical?"));
    for(size_t i = 0; i<proj->filenames.size();i++){
        ui->covariate_tableWidget->insertRow(ui->covariate_tableWidget->rowCount());
        ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
                                                0,
                                                new QTableWidgetItem(proj->filenames[i].c_str()));
        ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
                                                1,
                                                new QTableWidgetItem(proj->layernames[i].c_str()));
        QCheckBox *categoriacl_cb = new QCheckBox();
        categoriacl_cb->setChecked(false);
        if(proj->layertypes[i]=="CATEGORICAL") categoriacl_cb->setChecked(true);
        ui->covariate_tableWidget->setCellWidget(ui->covariate_tableWidget->rowCount()-1,
                                                2,
                                                categoriacl_cb);
    }
    if(mode==AddPrototypeBase::SAMPLE){
        ui->radioButton_poly->setVisible(false);
        ui->radioButton_soiltype->setVisible(false);
        ui->btn_hint->setVisible(false);
        ui->label_singlemode->setVisible(false);
        ui->checkBox->setVisible(false);
    }
    else if(mode==AddPrototypeBase::MAP){
        ui->radioButton_poly->setChecked(true);
        ui->label_2->setText("Select soil ID field:");
        ui->label->setText("Soil Map File");
        ui->label_3->setVisible(false);
        ui->yFiled_comboBox->setVisible(false);
        setWindowTitle("Get Prototype base from map");
    }
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

AddPrototypeBase::~AddPrototypeBase()
{
    delete ui;
}

void AddPrototypeBase::on_addCovariate_btn_clicked()
{
//    QStringList filenames = QFileDialog::getOpenFileNames(this,
//                                                   tr("Open environmental covariate file"),
//                                                   "./",
//                                                   tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
//    if(filenames.size()==0) return;
//    for(QString filename : filenames){
//        ui->covariate_tableWidget->insertRow(ui->covariate_tableWidget->rowCount());
//        ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
//                                                0,
//                                                new QTableWidgetItem(filename));
//        std::size_t first = filename.toStdString().find_last_of('/');
//        if (first==std::string::npos){
//            first = filename.toStdString().find_last_of('\\');
//        }
//        std::size_t end = filename.toStdString().find_last_of('.');
//        QString covariate = filename.toStdString().substr(first+1,end-first-1).c_str();
//        ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
//                                                1,
//                                                new QTableWidgetItem(covariate));
//        QCheckBox *categoriacl_cb = new QCheckBox();
//        categoriacl_cb->setChecked(false);
//        ui->covariate_tableWidget->setCellWidget(ui->covariate_tableWidget->rowCount()-1,
//                                                2,
//                                                categoriacl_cb);
//    }
    SimpleDialog addGisData(SimpleDialog::ADDGISDATA,project,this);
    addGisData.exec();
    for(int i = 0;i<ui->covariate_tableWidget->rowCount();i++){
        if(ui->covariate_tableWidget->item(i,0)->text()==addGisData.filename){
            QMessageBox warning;
            warning.setText("This file already exists in GIS data.");
            warning.exec();
            return;
        }
        if(ui->covariate_tableWidget->item(i,1)->text()==addGisData.covariate){
            QMessageBox warning;
            warning.setText("This covariate already exists in covariates. Please rename the covariate.");
            warning.exec();
            return;
        }
    }
    /*for(int i = 0;i<project->noFileLayers.size();i++){
        if(project->noFileLayers[i]==addGisData.covariate.toStdString()){
            project->noFileLayers.erase(project->noFileLayers.begin()+i);
            project->noFileLayers.shrink_to_fit();
            project->noFileDatatypes.erase(project->noFileDatatypes.begin()+i);
            project->noFileDatatypes.shrink_to_fit();
        }
    }
    project->layertypes.push_back(addGisData.datatype);
    project->layernames.push_back(addGisData.covariate.toStdString());
    project->filenames.push_back(addGisData.filename.toStdString());*/
    ui->covariate_tableWidget->insertRow(ui->covariate_tableWidget->rowCount());
    ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
                                            0,
                                            new QTableWidgetItem(addGisData.filename));
    ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
                                            1,
                                            new QTableWidgetItem(addGisData.covariate));
    QCheckBox *categoriacl_cb = new QCheckBox();
    categoriacl_cb->setChecked(false);
    if(addGisData.datatype=="CATEGORICAL") categoriacl_cb->setChecked(true);
    ui->covariate_tableWidget->setCellWidget(ui->covariate_tableWidget->rowCount()-1,
                                            2,
                                            categoriacl_cb);
}

void AddPrototypeBase::on_deleteCovariate_btn_clicked()
{
    ui->covariate_tableWidget->removeRow(ui->covariate_tableWidget->currentRow());
}

void AddPrototypeBase::on_browseSampleFile_btn_clicked()
{
    QString filename;
    if(mode==AddPrototypeBase::SAMPLE){
        filename = QFileDialog::getOpenFileName(this,
                                               tr("Open samples file"),
                                               project->workingDir,
                                               tr("Sample file(*.csv *.txt)"));
        if(filename.isEmpty()) return;
        ui->sampleFile_lineEdit->setText(filename);
        project->workingDir=QFileInfo(filename).absoluteDir().absolutePath();
        ifstream file(filename.toStdString()); // declare file stream:
        if(!file.is_open()) return;
        string line;
        getline(file, line);
        vector<string> names;
        solim::ParseStr(line, ',', names);
        QStringList columnNames;
        for(size_t i = 0;i<names.size();i++){
            columnNames.append(names[i].c_str());
        }
        ui->xFiled_comboBox->addItems(columnNames);
        ui->yFiled_comboBox->addItems(columnNames);
        for (size_t i = 0;i<names.size();i++){
            if(names[i]=="x"||names[i]=="X"){
                ui->xFiled_comboBox->setCurrentText(names[i].c_str());
            }
            if(names[i]=="y"||names[i]=="Y"){
                ui->yFiled_comboBox->setCurrentText(names[i].c_str());
            }
        }
    } else if (mode==AddPrototypeBase::MAP){
        filename = QFileDialog::getOpenFileName(this,
                                               tr("Open samples file"),
                                               project->workingDir,
                                               tr("Sample file(*.shp)"));
        if(filename.isEmpty()) return;
        ui->sampleFile_lineEdit->setText(filename);
        GDALAllRegister();
        GDALDataset *poDS;
        vector<string> soilIDs;
        poDS = (GDALDataset*)GDALOpenEx(filename.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
        if (poDS == NULL)
        {
            warn("File open failed.");
            ui->sampleFile_lineEdit->clear();
            return;
        }
        OGRLayer  *poLayer;
        poLayer = poDS->GetLayer(0);
        // check if shapefile type is polygon
        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
        if (poFDefn->GetGeomType() != wkbPolygon) {
            warn("Feature type is not polygon type. Cannot be used for data mining.");
            ui->sampleFile_lineEdit->clear();return;
        }
        QStringList fieldnames;
        for(int i = 0; i< poFDefn->GetFieldCount(); ++i) {
            fieldnames.push_back(poFDefn->GetFieldDefn(i)->GetNameRef());
        }
        ui->xFiled_comboBox->addItems(fieldnames);
    }
    int first = filename.lastIndexOf('/');
    if (first == -1){
        first = filename.lastIndexOf('\\');
    }
    int end = filename.lastIndexOf('.');
    QString prototypeBaseName = filename.mid(first+1,end-first-1);
    ui->lineEdit_basename->setText(prototypeBaseName);
}

void AddPrototypeBase::on_ok_btn_clicked()
{
    string sampleFile = ui->sampleFile_lineEdit->text().toStdString();
    if(sampleFile.empty()||ui->covariate_tableWidget->rowCount()==0) {
        ui->progressBar->setVisible(false);
        QMessageBox warning;
        warning.setText("Please put in sample file and/or covariate layers!");
        warning.setStandardButtons(QMessageBox::Ok);
        warning.exec();
        return;
    }
    string prototypeBaseName = ui->lineEdit_basename->text().toStdString();
    for(int i = 0; i< project->prototypeBaseNames.size();i++){
        if(prototypeBaseName==project->prototypeBaseNames[i]){
            QMessageBox warning;
            warning.setText("The sample file has been added, cannot add samples dupplicatedly!");
            warning.setStandardButtons(QMessageBox::Ok);
            warning.exec();
            ui->sampleFile_lineEdit->clear();
            ui->xFiled_comboBox->clear();
            ui->yFiled_comboBox->clear();
            ui->progressBar->setVisible(false);
            return;
        }
    }
    ui->ok_btn->setEnabled(false);
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    vector<string> envFileNames;
    vector<string> layernames;
    vector<string> datatypes;

    for(int i = 0; i<ui->covariate_tableWidget->rowCount();i++){
        envFileNames.push_back(ui->covariate_tableWidget->item(i,0)->text().toStdString());
        layernames.push_back(ui->covariate_tableWidget->item(i,1)->text().toStdString());
        QCheckBox *type = (QCheckBox*) ui->covariate_tableWidget->cellWidget(i,2);
        if(type->isChecked()==true){
            datatypes.push_back("CATEGORICAL");
        } else {
            datatypes.push_back("CONTINUOUS");
        }
    }
    solim::EnvDataset *eds = new solim::EnvDataset(envFileNames,datatypes,layernames);
    if(eds->Layers.size()<envFileNames.size()){
        QMessageBox inconsistentWarn;
        inconsistentWarn.setText("File size does not match. Not all layers added.");
        inconsistentWarn.exec();
        vector<string> newEnvFileNames;
        vector<string> newLayernames;
        vector<string> newDatatypes;
        for(int i = 0; i<eds->Layers.size();i++){
            for(int j = 0; j< envFileNames.size();j++){
                if(layernames[j]==eds->Layers[i]->LayerName){
                    newEnvFileNames.push_back(envFileNames[j]);
                    newLayernames.push_back(layernames[j]);
                    newDatatypes.push_back(datatypes[j]);
                }
            }
        }
        envFileNames = newEnvFileNames;
        layernames = newLayernames;
        datatypes = newDatatypes;
    }
    if(mode == AddPrototypeBase::SAMPLE){
        ui->progressBar->setValue(1);
        //ui->progressBar->setRange(0,0);
        vector<Prototype>* prototypes = Prototype::getPrototypesFromSample(sampleFile,eds, prototypeBaseName,
                                                                           ui->xFiled_comboBox->currentText().toStdString(),
                                                                           ui->yFiled_comboBox->currentText().toStdString());
        //project->prototypeBaseNames.push_back(prototypeBaseName);
        int protoNum=prototypes->size();
        if(protoNum>0){
            project->prototypeBaseNames.push_back(prototypeBaseName);
            vector<string> ids;
            for(int i =0; i<protoNum; i++){
                ids.push_back(prototypes->at(i).prototypeID);
            }
            std::sort(ids.begin(),ids.end());
            for(int i = 0; i<protoNum;i++){
                for(vector<Prototype>::iterator it=prototypes->begin();it!=prototypes->end();it++){
                    if((*it).prototypeID==ids[i])
                        project->prototypes.insert(project->prototypes.end(),it,it+1);
                }
            }
            //project->prototypes.insert(project->prototypes.end(),prototypes->begin(),prototypes->end());
        } else {
            QMessageBox warning;
            warning.setText("Samples are not within the range of covariate coordinates. Please check the coordinates of samples or covariates.");
            warning.exec();
        }
        ui->progressBar->setValue(2);
    } else if(mode == AddPrototypeBase::MAP){
        if(ui->radioButton_soiltype->isChecked()){
            vector<Prototype>* prototypes = Prototype::getPrototypesFromMining_soilType(sampleFile,eds,
                                                                                        ui->xFiled_comboBox->currentText().toStdString(),
                                                                                        prototypeBaseName,ui->progressBar);
            int protoNum=prototypes->size();
            if(protoNum>0){
                project->prototypeBaseNames.push_back(prototypeBaseName);
                vector<string> ids;
                for(int i =0; i<protoNum; i++){
                    ids.push_back(prototypes->at(i).prototypeID);
                }
                std::sort(ids.begin(),ids.end());
                for(int i = 0; i<protoNum;i++){
                    for(vector<Prototype>::iterator it=prototypes->begin();it!=prototypes->end();it++){
                        if((*it).prototypeID==ids[i])
                            project->prototypes.insert(project->prototypes.end(),it,it+1);
                    }
                }
                //project->prototypes.insert(project->prototypes.end(),prototypes->begin(),prototypes->end());
            }
        } else {// if(ui->radioButton_poly->isChecked()){
            vector<Prototype>* prototypes = Prototype::getPrototypesFromMining_polygon(sampleFile,eds,
                                                                                       ui->xFiled_comboBox->currentText().toStdString(),
                                                                                       prototypeBaseName,ui->progressBar);

            int protoNum=prototypes->size();
            if(protoNum>0){
                project->prototypeBaseNames.push_back(prototypeBaseName);
                vector<string> ids;
                for(int i =0; i<protoNum; i++){
                    ids.push_back(prototypes->at(i).prototypeID);
                }
                std::sort(ids.begin(),ids.end());
                for(int i = 0; i<protoNum;i++){
                    for(vector<Prototype>::iterator it=prototypes->begin();it!=prototypes->end();it++){
                        if((*it).prototypeID==ids[i])
                            project->prototypes.insert(project->prototypes.end(),it,it+1);
                    }
                }
                //project->prototypes.insert(project->prototypes.end(),prototypes->begin(),prototypes->end());
            }
        }
    }
    close();
}

void AddPrototypeBase::on_btn_hint_clicked()
{
    QMessageBox msg;
    msg.setTextFormat(Qt::RichText);
    msg.setTextInteractionFlags(Qt::TextBrowserInteraction);
    msg.setText("<b>Polygon mode</b>:"
                "<ul><li>When each polygon has <b>only one dominant soil type</b>: For each soil type, mining rules from every polygon, and then merge the rules from polygons to generate the rule for the soil type"
                " <a href=\"https://www.sciencedirect.com/science/article/pii/S2095311918619380\">(Cheng et al., 2019)</a>.</li>"
                "<li>When each polygon contains <b>one or more soil types</b>: To be continued</li></ul>"
                "<b>Soil Type mode</b>: "
                "<ul><li>When each polygon has <b>only one dominant soil type</b>: For each soil type, mining rules from overall distribution of covariate values from all polygon in the soil type"
                " <a href=\"https://www.tandfonline.com/doi/abs/10.1080/13658810310001596049\">(Qi and Zhu, 2003)</a>.</li>"
                "<li>When each polygon contains <b>one or more soil types</b>: To be continued</li></ul>");
    msg.setStandardButtons(QMessageBox::Ok);
    //ui->label_hint->setOpenExternalLinks(true);
    msg.exec();
}
