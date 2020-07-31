#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "soilinferencefromsamples.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionFrom_Samples, SIGNAL(triggered()), this, SLOT(onSoilInferenceFromSample()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSoilInferenceFromSample(){
    //soilInferenceFromSamples inferFromSamples;
    //inferFromSamples.setModal(true);
    //inferFromSamples.exec();
    inferFromSamples = new soilInferenceFromSamples(this);
    inferFromSamples->show();
}
