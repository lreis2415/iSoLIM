#include "prototypefromsamples.h"
#include "ui_prototypefromsamples.h"

prototypeFromSamples::prototypeFromSamples(SoLIMProject *proj,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::prototypeFromSamples)
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
}

prototypeFromSamples::~prototypeFromSamples()
{
    delete ui;
}

void prototypeFromSamples::on_addCovariate_btn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open environmental covariate file"),
                                                   "./",
                                                   tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    if(filename.isEmpty()) return;
    ui->covariate_tableWidget->insertRow(ui->covariate_tableWidget->rowCount());
    ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
                                            0,
                                            new QTableWidgetItem(filename));
    std::size_t first = filename.toStdString().find_last_of('/');
    if (first==std::string::npos){
        first = filename.toStdString().find_last_of('\\');
    }
    std::size_t end = filename.toStdString().find_last_of('.');
    QString covariate = filename.toStdString().substr(first+1,end-first-1).c_str();
    ui->covariate_tableWidget->setItem(ui->covariate_tableWidget->rowCount()-1,
                                            1,
                                            new QTableWidgetItem(covariate));
    QCheckBox *categoriacl_cb = new QCheckBox();
    categoriacl_cb->setChecked(false);
    ui->covariate_tableWidget->setCellWidget(ui->covariate_tableWidget->rowCount()-1,
                                            2,
                                            categoriacl_cb);
}

void prototypeFromSamples::on_deleteCovariate_btn_clicked()
{
    ui->covariate_tableWidget->removeRow(ui->covariate_tableWidget->currentRow());
}

void prototypeFromSamples::on_browseSampleFile_btn_clicked()
{
    QString samplesFileName = QFileDialog::getOpenFileName(this,
                                                           tr("Open samples file"),
                                                           "./",
                                                           tr("Sample file(*.csv *.txt)"));
    ui->sampleFile_lineEdit->setText(samplesFileName);
    QFile sampleFile(samplesFileName);
    if(!sampleFile.open(QIODevice::ReadOnly))
         qDebug()<<"OPEN FILE FAILED";
    QTextStream *out = new QTextStream(&sampleFile);//文本流
    QStringList tempOption = out->readAll().split("\n");//每行以\n区分
    QStringList columnNames = tempOption.at(0).split(",");
    ui->xFiled_comboBox->addItems(columnNames);
    ui->yFiled_comboBox->addItems(columnNames);
    for (int i = 0;i<columnNames.size();i++){
        QString name = columnNames.at(i);
        if(name.compare("x")==0||name.compare("X")==0){
            ui->xFiled_comboBox->setCurrentText(name);
        }
        if(name.compare("y")==0||name.compare("Y")==0){
            ui->yFiled_comboBox->setCurrentText(name);
        }
    }
}

void prototypeFromSamples::on_ok_btn_clicked()
{
    string sampleFile = ui->sampleFile_lineEdit->text().toStdString();
    if(sampleFile.empty()||ui->covariate_tableWidget->rowCount()==0){
        ui->progressBar->setVisible(false);
        QMessageBox warning;
        warning.setText("Please put in sample file and/or covariate layers!");
        warning.setStandardButtons(QMessageBox::Ok);
        warning.exec();
        return;
    }
    std::size_t first = sampleFile.find_last_of('/');
    if (first==std::string::npos){
        first = sampleFile.find_last_of('\\');
    }
    std::size_t end = sampleFile.find_last_of('.');
    string prototypeName = sampleFile.substr(first+1,end-first-1);
    for(int i = 0; i< project->prototypeBaseNames.size();i++){
        if(prototypeName==project->prototypeBaseNames[i]){
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
    ui->progressBar->setValue(1);
    //ui->progressBar->setRange(0,0);
    vector<Prototype>* prototypes = Prototype::getPrototypesFromSample(sampleFile,eds, prototypeName,
                                                                       ui->xFiled_comboBox->currentText().toStdString(),
                                                                       ui->yFiled_comboBox->currentText().toStdString());
    project->prototypeBaseNames.push_back(prototypeName);
    project->prototypes.insert(project->prototypes.end(),prototypes->begin(),prototypes->end());
    project->filenames.insert(project->filenames.end(),envFileNames.begin(),envFileNames.end());
    project->layernames.insert(project->layernames.end(),layernames.begin(),layernames.end());
    project->layertypes.insert(project->layertypes.end(),datatypes.begin(),datatypes.end());
    ui->progressBar->setValue(2);
    QFile sampleQfile(sampleFile.c_str());
    if(!sampleQfile.open(QIODevice::ReadOnly))
         qDebug()<<"OPEN FILE FAILED";
    QTextStream *out = new QTextStream(&sampleQfile);
    QStringList tempOption = out->readAll().split("\n");
    QStringList columnNames = tempOption.at(0).split('\r').at(0).split(",");
    for(QString column:columnNames){
        if(column.compare(ui->xFiled_comboBox->currentText())==0
                ||column.compare(ui->yFiled_comboBox->currentText())==0
                ||column.compare("ID",Qt::CaseInsensitive)==0)
            continue;
        project->propertyNames.push_back(column.toStdString());
    }
    close();
}
