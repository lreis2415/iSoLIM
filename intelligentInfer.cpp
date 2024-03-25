#include "intelligentInfer.h"
#include "ui_intelligentInfer.h"

//#define EXPERIMENT

intelligentInfer::intelligentInfer(SoLIMProject *proj, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::intelligentInfer),
    project(proj)
{
    setWindowFlags(Qt::Window
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint);
    show();
    setWindowIcon(QIcon("./imgs/solim.jpg"));
    outputAutoFill = true;
    project = proj;
    ui->setupUi(this);
    init = true;
    categoricalProps = QStringList();
    ui->progressBar->setVisible(false);
    ui->RepresentativeSample_checkbx->setChecked(true);
    if(proj==nullptr){

        QString studyArea = "";
        proj = new SoLIMProject();
        proj->projFilename = "./Untitled.slp";
        proj->projName = "Untitled";
        proj->studyArea="";
        workingDir = "./";
        proj->workingDir=workingDir;
    } else {
        workingDir = proj->workingDir;
    }
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

intelligentInfer::~intelligentInfer()
{
    delete ui;
}

void intelligentInfer::on_Inference_OK_btn_clicked()
{
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

    string targetName = ui->InferedProperty_comboBox->currentText().toStdString();
    string xField = ui->xFiled_comboBox->currentText().toStdString();
    string yField = ui->yFiled_comboBox->currentText().toStdString();
    string output = ui->OutputFile_lineEdit->text().toStdString();
    QFile output_img((output+".png").c_str());
    if(output_img.exists()) output_img.remove();
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(TRUE);
    ui->cancel_btn->setEnabled(false);
    ui->Inference_OK_btn->setEnabled(false);
    string sampleFile = ui->SampleFile_lineEdit->text().toStdString();

    /* todo
     * inference on python
    */
    QMessageBox msg;
    msg.setText("The automatically build model for spatial prediction is Random Forest. Click 'OK' to execute the model.");
    msg.exec();


    this->close();
}

void intelligentInfer::on_cancel_btn_clicked()
{
    this->close();
}

void intelligentInfer::tableItemClicked(int row,int col){
    if(col == 0){
        SimpleDialog addGisData(SimpleDialog::MODIFYGISDATA,project, this);
        addGisData.exec();
        ui->CovariateFiles_tableWidget->setItem(row,
                                                0,
                                                new QTableWidgetItem(addGisData.filename));
    }
}


void intelligentInfer::on_SampleFile_btn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                           tr("Open samples file"),
                                           workingDir,
                                           tr("Sample file(*.csv *.txt)"));
    if(filename.isEmpty()) return;
    ui->SampleFile_lineEdit->setText(filename);
    workingDir=QFileInfo(filename).absoluteDir().absolutePath();
    QTextCodec *code = QTextCodec::codecForName("UTF-8");

    //std::string filename_str = code->fromUnicode(filename.toStdString().c_str()).data();
    QString filename1 = QString::fromStdString(code->fromUnicode(filename).data());
    QFile file(filename1);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        warn("Cannot open sample file");
        ui->SampleFile_lineEdit->clear();
        return;
    }
    QTextStream txtInput(&file);
    string line= txtInput.readLine().toStdString();

    vector<string> names;
    solim::ParseStr(line, ',', names);
    QStringList columnNames;
    for(size_t i = 0;i<names.size();i++){
        columnNames.append(names[i].c_str());
    }
    categoricalProps.clear();
    ui->InferedProperty_comboBox->addItems(columnNames);
    ui->InferedProperty_comboBox->setEnabled(true);
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
}


void intelligentInfer::on_AddCov_btn_clicked()
{
    /*SimpleDialog addGisData(SimpleDialog::ADDGISDATA,project, this);
    addGisData.exec();
    if(addGisData.filename.isEmpty()){
        return;
    }
    if(project!=nullptr){
        for(size_t i = 0;i<project->filenames.size();i++){
            if(project->filenames[i]==addGisData.filename.toStdString()){
                QMessageBox warning;
                warning.setText("This file already exists in covariates.");
                warning.exec();
                return;
            }
            if(project->layernames[i]==addGisData.covariate.toStdString()){
                QMessageBox warning;
                warning.setText("This covariate name already exists. Please rename the covariate.");
                warning.exec();
                return;
            }
        }
        project->filenames.push_back(addGisData.filename.toStdString());
        project->layernames.push_back(addGisData.covariate.toStdString());
        project->layertypes.push_back(addGisData.datatype);
        project->layerDataMax.push_back(NODATA);
        project->layerDataMin.push_back(NODATA);
    }
    ui->CovariateFiles_tableWidget->setRowCount(ui->CovariateFiles_tableWidget->rowCount()+1);
    ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            0,
                                            new QTableWidgetItem(addGisData.filename));
    ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            1,
                                            new QTableWidgetItem(addGisData.covariate));
    QCheckBox *categoriacl_cb = new QCheckBox();
    if(addGisData.datatype=="CATEGORICAL")
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
                                                  selected_cb);*/

    QString filename = QFileDialog::getOpenFileName(this,
                                           tr("Open covariate file"),
                                           workingDir,
                                           tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    if(filename.isEmpty()) return;

    std::size_t first = filename.toStdString().find_last_of('/');
    if (first==std::string::npos){
        first = filename.toStdString().find_last_of('\\');
    }
    std::size_t end = filename.toStdString().find_last_of('.');
    QString covariate = filename.toStdString().substr(first+1,end-first-1).c_str();

    ui->CovariateFiles_tableWidget->setRowCount(ui->CovariateFiles_tableWidget->rowCount()+1);
    ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            0,
                                            new QTableWidgetItem(filename));
    ui->CovariateFiles_tableWidget->setItem(ui->CovariateFiles_tableWidget->rowCount()-1,
                                            1,
                                            new QTableWidgetItem(covariate));
    QCheckBox *categoriacl_cb = new QCheckBox();
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


void intelligentInfer::on_OutputFile_btn_clicked()
{

}

