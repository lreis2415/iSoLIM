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

    prototypeMenu = new QMenu(ui->dataTreeView);
    addPrototype = new QAction("Create new prototype base",prototypeMenu);
    prototypeMenu->addAction(addPrototype);
    ui->dataTreeView->addAction(addPrototype);
    connect(addPrototype,SIGNAL(triggered()),this,SLOT(onAddPrototype()));
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

    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(0,0,prototypeChild);
    resultChild = new QStandardItem("Results");
    model->item(0,0)->setChild(1,0,resultChild);
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
    proj = new SoLIMProject();
    proj->workingDiret = workingDirec;
    proj->projName = projNew.projName.toStdString();
    proj->filenames.clear();
    proj->layernames.clear();
    proj->prototypes.clear();
    proj->exceptions.clear();
}

void MainWindow::onProjectSave(){
    string filename = proj->workingDiret;
    if(filename.find("//")!=std::string::npos){
        filename += "//" + proj->projName+".slp";
    } else if(filename.find("\\\\")!=std::string::npos){
        filename+="\\\\" + proj->projName+".slp";
    }else if(filename.find('\\')!=std::string::npos){
        filename+='\\' + proj->projName+".slp";
    } else{
        filename+='/' + proj->projName+".slp";
    }

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc->LinkEndChild(pDeclaration);
    TiXmlElement *root_node = new TiXmlElement("Project");
    root_node->SetAttribute("Name", proj->projName.c_str());
    doc->LinkEndChild(root_node);
    TiXmlElement *workingDirec_node = new TiXmlElement("WorkingDirectory");
    root_node->LinkEndChild(workingDirec_node);
    TiXmlText *workingDirec_text = new TiXmlText(proj->workingDiret.c_str());
    workingDirec_node->LinkEndChild(workingDirec_text);
    TiXmlElement *gisData_node = new TiXmlElement("GISData");
    root_node->LinkEndChild(gisData_node);
    TiXmlElement *prototypes_node = new TiXmlElement("Prototypes");
    root_node->LinkEndChild(prototypes_node);
    for (int i = 0; i<proj->filenames.size();i++) {
        // add layer to GISData
        TiXmlElement *layer_node = new TiXmlElement("Layer");
        layer_node->SetAttribute("Name",proj->layernames[i].c_str());
        gisData_node->LinkEndChild(layer_node);

        TiXmlText *layer_text = new TiXmlText(proj->filenames[i].c_str());
        layer_node->LinkEndChild(layer_text);
    }
    for(vector<solim::Prototype>::iterator it = proj->prototypes.begin();it!=proj->prototypes.end();it++){
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
    proj = new SoLIMProject();
    proj->projName = projectHandle.ToElement()->Attribute("Name");
    proj->workingDiret = projectHandle.FirstChildElement("WorkingDirectory").ToElement()->GetText();

    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setData(model->index(0,0), QString::fromStdString(proj->projName));

    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(0,0,prototypeChild);
    resultChild = new QStandardItem("Results");
    model->item(0,0)->setChild(1,0,resultChild);

    model->setHorizontalHeaderItem( 0, new QStandardItem("Projects") );

    ui->dataTreeView->setModel( model );
    ui->dataTreeView->expand(model->item(0,0)->index());

    TiXmlHandle gisDataHandle = projectHandle.FirstChildElement("GISData");
    for(TiXmlElement* layer = gisDataHandle.FirstChildElement("Layer").ToElement();
        layer; layer = layer->NextSiblingElement("Layer")){
        string layername = layer->Attribute("Name");
        proj->layernames.push_back(layername);
        proj->filenames.push_back(layer->GetText());
    }
    // add prototypes
    TiXmlHandle prototypesHandle = projectHandle.FirstChildElement("Prototypes");
    for(TiXmlElement* prototype = prototypesHandle.FirstChildElement("Prototype").ToElement();
        prototype; prototype = prototype->NextSiblingElement()){
        Prototype proto;
        proto.prototypeBaseName = prototype->Attribute("BaseName");
        proto.prototypeID = prototype->Attribute("ID");
        TiXmlElement* envConditons_node = prototype->FirstChildElement("CurveLib");
        for(TiXmlElement *envAttri = envConditons_node->FirstChildElement("EnvAttri");
            envAttri; envAttri = envAttri->NextSiblingElement()){
            TiXmlElement *curveElement = envAttri->FirstChildElement("Curve");
            string source = curveElement->Attribute("Source");
            string covName = envAttri->Attribute("Name");
            solim::DataTypeEnum datatype = solim::getDatatypeFromString(curveElement->FirstChildElement("DataType")->GetText());
            int nodeNum = atoi(curveElement->FirstChildElement("NodeNum")->GetText());
            string coords = curveElement->FirstChildElement("Coordinates")->GetText();
            solim::Curve *c = new solim::Curve(covName, datatype, nodeNum, coords, source);
            c->typicalValue = atof(envAttri->FirstChildElement("TypicalValue")->GetText());
            proto.envConditions.push_back(*c);
            ++(proto.envConditionSize);
        }

        TiXmlElement *props = prototype->FirstChildElement("PropertyLib");
        for (TiXmlElement* prop = props->FirstChildElement("Property");
            prop; prop = prop->NextSiblingElement("Property")) {
            solim::SoilProperty p;
            p.propertyName = prop->Attribute("Name");
            p.propertyValue = atof(prop->GetText());
            p.soilPropertyType = solim::getDatatypeFromString(prop->Attribute("Type"));
            proto.properties.push_back(p);
        }
        proj->prototypes.push_back(proto);
    }
    onGetPrototype();
}
void MainWindow::onSoilInferenceFromSample(){
    inferFromSamples = new soilInferenceFromSamples(*proj, this);
   // inferFromSamples->setProj(proj);
    inferFromSamples->show();
}

void MainWindow::onAddPrototype(){
    getPrototype = new prototypeFromSamples(proj,this);
    getPrototype->show();
    // show prototypes
    connect(getPrototype,SIGNAL(finished(int)),this,SLOT(onGetPrototype()));
}

void MainWindow::onGetPrototype(){
    prototypeChild->setColumnCount(1);
    if(proj->prototypes.size()>0){
        for(vector<Prototype>::iterator it = proj->prototypes.begin(); it!=proj->prototypes.end();it++){
            prototypeChild->setRowCount(prototypeChild->rowCount()+1);
            string prototypename = "Prototype: "+(*it).prototypeBaseName +" (ID: "+(*it).prototypeID + ")";
            QStandardItem* prototype = new QStandardItem(prototypename.c_str());
            prototypeChild->setChild(prototypeChild->rowCount()-1,0,prototype);
            prototype->setColumnCount(1);
            prototype->setRowCount(2);
            QStandardItem* properties = new QStandardItem("Properties");
            prototype->setChild(0,0,properties);
            properties->setColumnCount(1);
            for(int i = 0;i<(*it).properties.size();i++) {
                string property = (*it).properties[i].propertyName;
                if((*it).properties[i].soilPropertyType==solim::CATEGORICAL){
                    property += " (category): " + to_string(int((*it).properties[i].propertyValue));
                } else
                    property += " (property): " + to_string((*it).properties[i].propertyValue);
                properties->setRowCount(properties->rowCount()+1);
                properties->setChild(properties->rowCount()-1,0,new QStandardItem(property.c_str()));
            }
            QStandardItem* covariates = new QStandardItem("Covariates");
            prototype->setChild(1,0,covariates);
            covariates->setColumnCount(1);
            for(int j = 0; j<(*it).envConditionSize;j++){
                string cov = "Covariate: " + (*it).envConditions[j].covariateName;
                covariates->setRowCount(covariates->rowCount()+1);
                QStandardItem *covItem = new QStandardItem(cov.c_str());
                covariates->setChild(covariates->rowCount()-1,0,covItem);
                covItem->setRowCount(4);
                covItem->setColumnCount(1);
                // set source child
                string source = "Source: ";
                source.append(solim::PrototypeSource_str[(*it).envConditions[j].source]);
                covItem->setChild(0,0,new QStandardItem(source.c_str()));
                // set data file child
                string datafile = "Data file: ";
                for(int k = 0;k < proj->layernames.size();k++){
                    if(strcmp((*it).envConditions[j].covariateName.c_str(),proj->layernames[k].c_str())==0){
                        datafile += proj->filenames[k];
                        break;
                    }
                }
                covItem->setChild(1,0,new QStandardItem(datafile.c_str()));
                // set typical value
                double typicalValue = (*it).envConditions[j].typicalValue;
                if(fabs(typicalValue-NODATA)>VERY_SMALL){
                    string typicalV = "Typical value: "+to_string(typicalValue);
                    covItem->setChild(2,0,new QStandardItem(typicalV.c_str()));
                } else {
                    covItem->setRowCount(3);
                }
                // set membership function
                covItem->setChild(covItem->rowCount()-1,0,new QStandardItem("Membership Function"));
            }
        }
    }
}


void MainWindow::onCustomContextMenu(const QPoint & point){
    QModelIndex index = ui->dataTreeView->indexAt(point);
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
