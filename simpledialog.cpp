#include "simpledialog.h"
#include "ui_simpledialog.h"

SimpleDialog::SimpleDialog(int mode, SoLIMProject *proj, QWidget *parent) :
    mode(mode),QDialog(parent),
    ui(new Ui::SimpleDialog)
{
    ui->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->label_hint->setFrameStyle(QFrame::Sunken);
    ui->label_hint->setTextFormat(Qt::RichText);
    ui->label_hint->setStyleSheet("QLabel { background-color : lightGray; color : black; }");
    ui->label_hint->setFrameStyle(QFrame::Box);
    ui->label_hint->setWordWrap(true);
    switch (mode) {
    case ADDGISDATA:
        ui->btn_2->setVisible(false);
        ui->next_btn->setVisible(false);
        ui->ok_btn->setEnabled(false);
        ui->label_hint->setVisible(false);
        ui->label_3->setVisible(false);
        ui->lineEdit_3->setVisible(false);
        ui->btn_3->setVisible(false);
        break;
    case MODIFYGISDATA:
        ui->btn_2->setVisible(false);
        ui->next_btn->setVisible(false);
        ui->ok_btn->setEnabled(false);
        ui->label_hint->setVisible(false);
        ui->label_2->setVisible(false);
        ui->lineEdit_2->setVisible(false);
        ui->checkBox->setVisible(false);
        setWindowTitle("Modify covariate file");
        ui->label_3->setVisible(false);
        ui->lineEdit_3->setVisible(false);
        ui->btn_3->setVisible(false);
        break;
    case ADDCOVARIATE:
        ui->label_1->setText("Covariate*:");
        ui->btn_1->setVisible(false);
        ui->label_2->setText("Filename:  ");
        ui->next_btn->setVisible(false);
        setWindowTitle("Add covariate");
        ui->label_hint->setVisible(true);
        ui->label_hint->setText("<b>Hint</b>: Covariate is required. Covaraite name can be derived from filename."
                                " You can also add a covariate without specifying corresponding filename.");
        ui->label_3->setVisible(false);
        ui->lineEdit_3->setVisible(false);
        ui->btn_3->setVisible(false);
        break;
    case ADDPROTOTYPEBASE:
        this->setWindowTitle("Create Prototype Base");
        ui->label_1->setVisible(false);
        ui->lineEdit_1->setVisible(false);
        ui->btn_1->setVisible(false);
        ui->btn_2->setVisible(false);
        ui->label_2->setText("Create Prototype Base");
        ui->checkBox->setVisible(false);  
        ui->label_hint->setVisible(false);
        ui->label_3->setVisible(false);
        ui->lineEdit_3->setVisible(false);
        ui->btn_3->setVisible(false);
        break;
    case EDITSTUDYAREA:
        this->setWindowTitle("Edit Study Area");
        ui->label_1->setVisible(false);
        ui->lineEdit_1->setVisible(false);
        ui->btn_1->setVisible(false);
        ui->btn_2->setVisible(false);
        ui->label_2->setText("Study Area Name");
        ui->lineEdit_2->setText(proj->studyArea.c_str());
        ui->checkBox->setVisible(false);
        ui->label_hint->setVisible(false);
        ui->next_btn->setVisible(false);
        ui->label_3->setVisible(false);
        ui->lineEdit_3->setVisible(false);
        ui->btn_3->setVisible(false);
        break;
    case RESAMPLE:
        ui->label_1->setText("Input File: ");
        ui->lineEdit_1->clear();
        ui->label_2->setText("Output File:");
        ui->lineEdit_2->clear();
        ui->checkBox->setVisible(false);
        ui->label_hint->setVisible(false);
        ui->next_btn->setVisible(false);
        break;
    }
    filename="";
    covariate="";
    lineEdit2="";
    nextFlag=false;
    workingDir=proj->workingDir;
    project=proj;
}

SimpleDialog::~SimpleDialog()
{
    delete ui;
}

void SimpleDialog::on_lineEdit_1_textChanged(const QString &arg1)
{
    if(mode==ADDGISDATA||MODIFYGISDATA){
        if(!arg1.isEmpty())
            ui->ok_btn->setEnabled(true);
    }
}

void SimpleDialog::on_btn_1_clicked()
{
    if(mode==ADDGISDATA||mode==MODIFYGISDATA){
        QString qfilename = QFileDialog::getOpenFileName(this,
                                                        tr("Open environmental covariate file"),
                                                        workingDir,
                                                        tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        if(!qfilename.isEmpty()) workingDir=QFileInfo(qfilename).absoluteDir().absolutePath();
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
    else if(mode==RESAMPLE){
        QString qfilename = QFileDialog::getOpenFileName(this,
                                                        tr("Open input file"),
                                                        workingDir,
                                                        tr("Raster file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        if(!qfilename.isEmpty()) workingDir=QFileInfo(qfilename).absoluteDir().absolutePath();
        ui->lineEdit_1->setText(qfilename);
        filename = qfilename;
        QString ext = QFileInfo(filename).suffix();
        QString save_filename = qfilename.left(qfilename.length()-ext.length()-1)+"_resample."+ext;
        int i = 1;
        while (QFileInfo(save_filename).exists()){
            save_filename = qfilename.left(qfilename.length()-ext.length()-1) + "_resample(" + QString::number(i) + ")."+ext;
        }
        ui->lineEdit_2->setText(save_filename);
    }
}

void SimpleDialog::on_btn_2_clicked(){
    if(mode==ADDCOVARIATE){
        QString qfilename = QFileDialog::getOpenFileName(this,
                                                        tr("Open environmental covariate file"),
                                                        workingDir,
                                                        tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        ui->lineEdit_2->setText(qfilename);
        if(!qfilename.isEmpty()) workingDir=QFileInfo(qfilename).absoluteDir().absolutePath();
        filename = qfilename;
        if(!filename.isEmpty()){
            std::size_t first = filename.toStdString().find_last_of('/');
            if (first==std::string::npos){
                first = filename.toStdString().find_last_of('\\');
            }
            std::size_t end = filename.toStdString().find_last_of('.');
            covariate = filename.toStdString().substr(first+1,end-first-1).c_str();
        }
        ui->lineEdit_1->setText(covariate);
    }
    if(mode==RESAMPLE){
        QString qfilename = QFileDialog::getSaveFileName(this,
                                                        tr("Open output file"),
                                                        workingDir,
                                                        tr("Raster file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        if(!qfilename.isEmpty()) workingDir=QFileInfo(qfilename).absoluteDir().absolutePath();
        ui->lineEdit_2->setText(qfilename);
    }
}

void SimpleDialog::on_cancel_btn_clicked()
{
    filename.clear();
    covariate.clear();
    lineEdit2.clear();
    project->workingDir=workingDir;
    close();
}

void SimpleDialog::on_ok_btn_clicked()
{
    project->workingDir=workingDir;
    if(mode==ADDGISDATA){
        filename = ui->lineEdit_1->text();
        covariate=ui->lineEdit_2->text();
        if(ui->checkBox->isChecked()) datatype="CATEGORICAL";
        else datatype="CONTINUOUS";
        if(QFileInfo(filename).exists()){
            close();
        } else {
            QMessageBox warn;
            warn.setText("File does not exist.");
            warn.exec();
        }
    } else if(mode==MODIFYGISDATA){
        filename = ui->lineEdit_1->text();
        if(QFileInfo(filename).exists()){
            close();
        } else {
            QMessageBox warn;
            warn.setText("File does not exist.");
            warn.exec();
        }
    } else if (mode==ADDCOVARIATE){
        filename = ui->lineEdit_2->text();
        covariate=ui->lineEdit_1->text();
        if(ui->checkBox->isChecked()) datatype="CATEGORICAL";
        else datatype="CONTINUOUS";
        if(!filename.isEmpty()){
            if(!QFileInfo(filename).exists()) filename="";
        }
        close();
    } else if(mode==ADDPROTOTYPEBASE){
        if(ui->lineEdit_2->text().isEmpty()){
            lineEdit2.clear();
            nextFlag=false;
            return;
        }
        else{
            lineEdit2=ui->lineEdit_2->text();
            close();
        }
    } else if(mode==EDITSTUDYAREA){
        lineEdit2=ui->lineEdit_2->text();
        close();
    } else if(mode==RESAMPLE){
        QString inputFile = ui->lineEdit_1->text();
        lineEdit2=ui->lineEdit_2->text();
        QString refFile = ui->lineEdit_3->text();
        if(QFileInfo(refFile).exists()&&QFileInfo(inputFile).exists()){
            lineEdit3 = refFile;
            filename = inputFile;
        } else {
            QMessageBox warn;
            warn.setText("Input File or Reference File does not exist.");
            warn.exec();
            return;
        }
        close();
    }
}

void SimpleDialog::on_next_btn_clicked(){
    nextFlag=true;
    on_ok_btn_clicked();
}

void SimpleDialog::on_btn_3_clicked(){
    if(mode==RESAMPLE){
        QString refFile = QFileDialog::getOpenFileName(this,
                                                        tr("Open environmental covariate file"),
                                                        workingDir,
                                                        tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
        if(!refFile.isEmpty()) workingDir = QFileInfo(refFile).absoluteDir().absolutePath();
        ui->lineEdit_3->setText(refFile);
        if(!refFile.isEmpty()) workingDir=QFileInfo(refFile).absoluteDir().absolutePath();
        ui->lineEdit_3->setText(refFile);

    }
}
