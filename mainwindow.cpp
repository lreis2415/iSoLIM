#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "soilinferencefromsamples.h"
#include "projectnew.h"
#include "QStandardItem"
#include "QStandardItemModel"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // setup main menu
    connect(ui->actionFrom_Samples, SIGNAL(triggered()), this, SLOT(onSoilInferenceFromSample()));
    connect(ui->actionNew,SIGNAL(triggered()),this,SLOT(onProjectNew()));
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(onProjectSave()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(onProjectOpen()));

    // setup project menu
    ui->dataTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->dataTreeView,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(onCustomContextMenu(const QPoint &)));
    gisDataMenu = new QMenu(ui->dataTreeView);
    addLayer = new QAction("Add Layer", gisDataMenu);
    gisDataMenu->addAction(addLayer);
    ui->dataTreeView->addAction(addLayer);
    connect(addLayer,SIGNAL(triggered()),this,SLOT(onAddLayer()));

    prototypeMenu = new QMenu(ui->dataTreeView);
    addPrototype = new QAction("Add Prototype",prototypeMenu);
    addExclusion = new QAction("Add Exclusion", prototypeMenu);
    addOccurrence = new QAction("Add Occurrence", prototypeMenu);
    QList<QAction*> prototypeActions;
    prototypeActions.push_back(addPrototype);
    prototypeActions.push_back(addExclusion);
    prototypeActions.push_back(addOccurrence);
    prototypeMenu->addActions(prototypeActions);
    ui->dataTreeView->addActions(prototypeActions);
    connect(addPrototype,SIGNAL(triggered()),this,SLOT(onAddPrototype()));
    connect(addExclusion,SIGNAL(triggered()),this,SLOT(onAddExclusion()));
    connect(addOccurrence,SIGNAL(triggered()),this,SLOT(onAddOccurrence()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProjectNew(){
    ProjectNew projNew;
    projNew.setModal(true);
    projNew.exec();

    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setData(model->index(0,0), projNew.projName);

    gisDataChild = new QStandardItem("GIS Data");
    model->item(0,0)->setChild(0,0,gisDataChild);
    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(1,0,prototypeChild);
    QStandardItem *resultChild = new QStandardItem("Results");
    model->item(0,0)->setChild(2,0,resultChild);
    model->setHorizontalHeaderItem( 0, new QStandardItem("Projects") );

    ui->dataTreeView->setModel( model );
    ui->dataTreeView->expandAll();

    string workingDirec = projNew.projDirec.toStdString();
    if(workingDirec.find("//")!=std::string::npos){
        workingDirec += "//" + projNew.projName.toStdString();
    } else if(workingDirec.find("\\\\")!=std::string::npos){
        workingDirec+="\\\\"+projNew.projName.toStdString();
    }else if(workingDirec.find('\\')!=std::string::npos){
        workingDirec+='\\'+projNew.projName.toStdString();
    } else{
        workingDirec+='/'+projNew.projName.toStdString();
    }
    if(!QDir(QString::fromStdString(workingDirec)).exists())
        QDir().mkdir(QString::fromStdString(workingDirec));
    proj.workingDiret = workingDirec;
    proj.projName = projNew.projName.toStdString();
    proj.filenames.clear();
    proj.layernames.clear();
    proj.prototypes.clear();
    proj.exceptions.clear();
}

void MainWindow::onProjectSave(){
    string filename = proj.workingDiret;
    if(filename.find("//")!=std::string::npos){
        filename += "//" + proj.projName+".slp";
    } else if(filename.find("\\\\")!=std::string::npos){
        filename+="\\\\" + proj.projName+".slp";
    }else if(filename.find('\\')!=std::string::npos){
        filename+='\\' + proj.projName+".slp";
    } else{
        filename+='/' + proj.projName+".slp";
    }

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc->LinkEndChild(pDeclaration);
    TiXmlElement *root_node = new TiXmlElement("Project");
    root_node->SetAttribute("Name", proj.projName.c_str());
    doc->LinkEndChild(root_node);
    TiXmlElement *workingDirec_node = new TiXmlElement("WorkingDirectory");
    root_node->LinkEndChild(workingDirec_node);
    TiXmlText *workingDirec_text = new TiXmlText(proj.workingDiret.c_str());
    workingDirec_node->LinkEndChild(workingDirec_text);
    TiXmlElement *gisData_node = new TiXmlElement("GISData");
    root_node->LinkEndChild(gisData_node);
    TiXmlElement *prototypes_node = new TiXmlElement("Prototypes");
    root_node->LinkEndChild(prototypes_node);
    for (int i = 0; i<proj.filenames.size();i++) {
        // add layer to GISData
        TiXmlElement *layer_node = new TiXmlElement("Layer");
        layer_node->SetAttribute("Name",proj.layernames[i].c_str());
        gisData_node->LinkEndChild(layer_node);

        TiXmlText *layer_text = new TiXmlText(proj.filenames[i].c_str());
        layer_node->LinkEndChild(layer_text);
    }
    for(vector<solim::Prototype>::iterator it = proj.prototypes.begin();it!=proj.prototypes.end();it++){
        prototypes_node->LinkEndChild((*it).writePrototypeXmlElement());
    }
    doc->SaveFile(filename.c_str());
}
void MainWindow::onProjectOpen(){
    QString projectFile = QFileDialog::getOpenFileName(this,
                                                       tr("Open SoLIM Project"),
                                                       "./",
                                                       tr("Project file(*.slp)"));
    TiXmlDocument doc(projectFile.toStdString().c_str());
    bool loadOK = doc.LoadFile();
    if (!loadOK) {
        throw invalid_argument("Failed to read xml file");
    }
    TiXmlHandle docHandle(&doc);
    TiXmlHandle projectHandle = docHandle.FirstChildElement("Project");
    proj.projName=projectHandle.ToElement()->Attribute("Name");
    proj.workingDiret = projectHandle.FirstChildElement("WorkingDirectory").ToElement()->GetText();
    proj.filenames.clear();
    proj.layernames.clear();
    proj.prototypes.clear();
    proj.exceptions.clear();

    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setData(model->index(0,0), QString::fromStdString(proj.projName));

    gisDataChild = new QStandardItem("GIS Data");
    model->item(0,0)->setChild(0,0,gisDataChild);
    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(1,0,prototypeChild);

    model->setHorizontalHeaderItem( 0, new QStandardItem("Projects") );

    ui->dataTreeView->setModel( model );
    ui->dataTreeView->expandAll();
    TiXmlHandle gisDataHandle = projectHandle.FirstChildElement("GISData");
    for(TiXmlElement* layer = gisDataHandle.FirstChildElement("Layer").ToElement();
        layer; layer = layer->NextSiblingElement("Layer")){
        string layername = layer->Attribute("Name");
        proj.layernames.push_back(layername);
        proj.filenames.push_back(layer->GetText());
        gisDataChild->setColumnCount(1);
        gisDataChild->setRowCount(gisDataChild->rowCount()+1);
        gisDataChild->setChild(gisDataChild->rowCount()-1,0,new QStandardItem(QString::fromStdString(layername)));
    }
    //TODO: add prototypes
}
void MainWindow::onSoilInferenceFromSample(){
    inferFromSamples = new soilInferenceFromSamples(proj, this);
   // inferFromSamples->setProj(proj);
    inferFromSamples->show();
}

void MainWindow::onAddLayer(){
    QString layerFileName = QFileDialog::getOpenFileName(this,
                                                     tr("Open environmental covariate file"),
                                                     "./",
                                                     tr("Covariate file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)"));
    std::size_t first = layerFileName.toStdString().find_last_of('/');
    if (first==std::string::npos){
        first = layerFileName.toStdString().find_last_of('\\');
    }
    std::size_t end = layerFileName.toStdString().find_last_of('.');
    string layerName = layerFileName.toStdString().substr(first+1,end-first-1);
    gisDataChild->setColumnCount(1);
    gisDataChild->setRowCount(gisDataChild->rowCount()+1);
    gisDataChild->setChild(gisDataChild->rowCount()-1,0,new QStandardItem(QString::fromStdString(layerName)));
    proj.filenames.push_back(layerFileName.toStdString());
    proj.layernames.push_back(layerName);
    drawLayer(layerFileName.toStdString());
}
void MainWindow::onAddPrototype(){}
void MainWindow::onAddExclusion(){}
void MainWindow::onAddOccurrence(){}

void MainWindow::onCustomContextMenu(const QPoint & point){
    QModelIndex index = ui->dataTreeView->indexAt(point);
    if(index.isValid()&&index.data().toString().compare("GIS Data")==0){
        gisDataMenu->exec(ui->dataTreeView->viewport()->mapToGlobal(point));
    }
    if(index.isValid()&&index.data().toString().compare("Prototypes")==0){
        prototypeMenu->exec(ui->dataTreeView->viewport()->mapToGlobal(point));
    }
}

void MainWindow::drawLayer(string filename){
    BaseIO *lyr = new BaseIO(filename);
    int viewWidth = ui->graphicsView->width();
    int viewHeight = ui->graphicsView->height();
    int lyrWidth = lyr->getXSize();
    int lyrHeight = lyr->getYSize();
    double stretchRatio;
    if (lyrHeight * viewWidth > viewHeight * lyrWidth) {
        stretchRatio = 1.0*lyrWidth/viewWidth;
    } else {
        stretchRatio = 1.0*lyrHeight/viewHeight;
    }
    if (stretchRatio>1){
        int shrink = stretchRatio;

    } else {
        int stretch = 1.0/stretchRatio;
    }
}
