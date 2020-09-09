#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->actionAdd_prototypes_from_samples->setDisabled(true);
    ui->menuCovariates->setDisabled(true);
    ui->menuSample_Design->setDisabled(true);
    setWindowIcon(QIcon("./imgs/solim.jpg"));
    // setup dock view
    initialProjectView();
    addDockWidget(Qt::LeftDockWidgetArea,projectDock);

    // setup data details dock
    initDataDetailsView();
    addDockWidget(Qt::LeftDockWidgetArea,dataDetailsDock);
    // setup main menu
    connect(ui->actionFrom_Prototypes, SIGNAL(triggered()), this, SLOT(onSoilInferenceFromPrototypes()));
    connect(ui->actionNew,SIGNAL(triggered()),this,SLOT(onProjectNew()));
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(onProjectSave()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(onProjectOpen()));
    connect(ui->actionAdd_prototypes_from_samples, SIGNAL(triggered()), this, SLOT(onAddPrototypeFromSamples()));
    connect(ui->actionView_Data,SIGNAL(triggered()),this,SLOT(onViewData()));
    projectViewInitialized = false;
    projectSaved = true;
    img = nullptr;
    proj = nullptr;
    myGraphicsView = new MyGraphicsView();
    ui->centralwidget->layout()->addWidget(myGraphicsView);
    myGraphicsView->dataDetailsView = dataDetailsView;
    zoomToolBar = addToolBar(tr("Zoom In"));
    const QIcon zoomInIcon = QIcon("./imgs/zoomin.svg");//QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *zoomInAct = new QAction(zoomInIcon, tr("&Zoom In"), this);
    zoomInAct->setStatusTip(tr("Zoom In"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(onZoomin()));
    zoomToolBar->addAction(zoomInAct);
    const QIcon zoomOutIcon = QIcon("./imgs/zoomout.svg");//QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *zoomOutAct = new QAction(zoomOutIcon, tr("&Zoom Out"), this);
    zoomOutAct->setStatusTip(tr("Zoom Out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(onZoomout()));
    zoomToolBar->addAction(zoomOutAct);
    zoomToolBar->setVisible(false);
    getPrototype = nullptr;
    prototypesFromSamples = nullptr;
    addPrototype = nullptr;
    addExclusion = nullptr;
    addOccurrence = nullptr;
    viewDataMenu = nullptr;
    viewData = nullptr;
    resultChild = nullptr;
    prototypeChild = nullptr;
}

MainWindow::~MainWindow()
{
    if(!projectSaved){
        saveWarning();
    }
    delete ui;
    if(img!=nullptr) delete img;
    if(projectView!=nullptr) delete projectView;
    if(projectDock!=nullptr) delete projectDock;
    if(getPrototype!=nullptr)    delete getPrototype;
    if(model!=nullptr)    delete model;
    if(prototypeMenu!=nullptr)   delete prototypeMenu;
    if(prototypesFromSamples!=nullptr)   delete prototypesFromSamples;
    if(addPrototype!=nullptr)    delete addPrototype;
    if(addExclusion!=nullptr)    delete addExclusion;
    if(addOccurrence!=nullptr)   delete addOccurrence;
    if(viewDataMenu!=nullptr)    delete viewDataMenu;
    if(viewData!=nullptr)    delete viewData;
    if(resultChild!=nullptr) delete resultChild;
    if(prototypeChild!=nullptr)  delete prototypeChild;
    if(proj!=nullptr)    delete proj;
}
void MainWindow::initDataDetailsView(){
    dataDetailsDock = new QDockWidget(tr("Data details"), this);
    dataDetailsView = new QTableView(dataDetailsDock);
    dataDetailsView->verticalHeader()->hide();
    dataDetailsView->horizontalHeader()->hide();
//    for(int i = 0; i<3;i++)
//        dataDetailsView->setColumnWidth(i,50);
    dataDetailsView->setShowGrid(false);
    dataDetailsDock->setFeatures(dataDetailsDock->features() & ~QDockWidget::DockWidgetClosable);
    dataDetailsDock->setWidget(dataDetailsView);
    dataDetailsDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::BottomDockWidgetArea);
}
void MainWindow::onProjectNew(){
    if(!saveWarning())
        return;
    string projFilename = QFileDialog::getSaveFileName(this,
                                                  tr("Create SoLIM Project"),
                                                  "./",
                                                  tr("Project file(*.slp)")).toStdString();
    if(projFilename.empty()){
        return;
    }
    string projName;
    size_t start = projFilename.find_last_of("/");
    if(start==std::string::npos)
        start = projFilename.find_last_of("\\");
    size_t end = projFilename.find_last_of(".");
    projName = projFilename.substr(start+1,end-start-1);


    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setData(model->index(0,0), projName.c_str());

    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(0,0,prototypeChild);
    resultChild = new QStandardItem("Results");
    model->item(0,0)->setChild(1,0,resultChild);
    model->setHorizontalHeaderItem( 0, new QStandardItem("Projects") );
    //
    //initialProjectView();
    projectView->setModel(model);
    projectView->expand(model->item(0,0)->index());

    proj = new SoLIMProject();
    proj->projFilename = projFilename;
    proj->projName = projName;
    projectSaved = false;
    connect(projectView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)));
    ui->actionAdd_prototypes_from_samples->setEnabled(true);
}

bool MainWindow::saveWarning(){
    if(!projectSaved){
        QMessageBox saveBox;
        saveBox.setText("Do you want to save the current project?");
        saveBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        saveBox.setDefaultButton(QMessageBox::Save);
        int ret = saveBox.exec();
        switch(ret){
        case QMessageBox::Save:
            onProjectSave();
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
            return false;
        default:
            onProjectSave();
        }
    }
    projectSaved = true;
    return true;
}

void MainWindow::initialProjectView(){
    if(projectViewInitialized){
        return;
    }
    projectDock = new QDockWidget(tr("Project"), this);
    projectView = new QTreeView(projectDock);
    //removeDockWidget(dataDetailsDock);
    projectDock->setFeatures(projectDock->features() & ~QDockWidget::DockWidgetClosable);
    projectDock->setWidget(projectView);
    //initDataDetailsView();
    //addDockWidget(Qt::LeftDockWidgetArea,dataDetailsDock);
    //myGraphicsView->dataDetailsView = dataDetailsView;
    projectView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(projectView,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(onCustomContextMenu(const QPoint &)));
    prototypeMenu = new QMenu(projectView);
    prototypesFromSamples = new QAction("Create new prototype base",prototypeMenu);
    prototypeMenu->addAction(prototypesFromSamples);
    projectView->addAction(prototypesFromSamples);
    viewDataMenu = new QMenu(projectView);
    viewData = new QAction("View Data", viewDataMenu);
    viewDataMenu->addAction(viewData);
    projectView->addAction(viewData);
    connect(prototypesFromSamples,SIGNAL(triggered()),this,SLOT(onAddPrototypeFromSamples()));
    projectView->addAction(viewData);
    projectViewInitialized = true;
}
void MainWindow::onSelectionChanged(const QItemSelection& current,const QItemSelection& previous){
    QModelIndex index = current.indexes().at(0);
    if(index.isValid()&&index.parent().data().toString().compare("Results")==0){
        string filename = index.data().toString().toStdString();
        drawLayer(filename);
    }
    else if(index.isValid()&&index.parent().data().toString().compare("Covariates")==0){
        string filename = index.child(1,0).data().toString().toStdString();
        filename = filename.substr(11);
        drawLayer(filename);
    }
    else if(index.isValid()&&index.data().toString().compare("Membership Function")==0){
        string prototype = index.parent().parent().parent().data().toString().toStdString();
        int prefixLength = 11;
        int idPrefixLength = 5;
        if(prototype.length()<prefixLength+idPrefixLength)
            return;
        prototype = prototype.substr(prefixLength);
        size_t first = prototype.find_first_of("(ID:");
        size_t last = prototype.find_last_of(")");
        string baseName = prototype.substr(0,first-1);
        string protoID = prototype.substr(first+idPrefixLength,last-first-idPrefixLength);
        string covName = index.parent().data().toString().toStdString().substr(prefixLength);
        drawMembershipFunction(baseName,protoID,covName);
    }
}

void MainWindow::onProjectSave(){
    if(!proj){
        return;
    }
    string filename = proj->projFilename;

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc->LinkEndChild(pDeclaration);
    TiXmlElement *root_node = new TiXmlElement("Project");
    root_node->SetAttribute("Name", proj->projName.c_str());
    doc->LinkEndChild(root_node);
    TiXmlElement *workingDirec_node = new TiXmlElement("ProjectFilename");
    root_node->LinkEndChild(workingDirec_node);
    TiXmlText *workingDirec_text = new TiXmlText(proj->projFilename.c_str());
    workingDirec_node->LinkEndChild(workingDirec_text);
    TiXmlElement *gisData_node = new TiXmlElement("GISData");
    root_node->LinkEndChild(gisData_node);
    TiXmlElement *prototypes_node = new TiXmlElement("Prototypes");
    root_node->LinkEndChild(prototypes_node);
    for (int i = 0; i<proj->filenames.size();i++) {
        // add layer to GISData
        TiXmlElement *layer_node = new TiXmlElement("Layer");
        layer_node->SetAttribute("Name",proj->layernames[i].c_str());
        layer_node->SetAttribute("Type",proj->layertypes[i].c_str());
        gisData_node->LinkEndChild(layer_node);

        TiXmlText *layer_text = new TiXmlText(proj->filenames[i].c_str());
        layer_node->LinkEndChild(layer_text);
    }
    for(vector<solim::Prototype>::iterator it = proj->prototypes.begin();it!=proj->prototypes.end();it++){
        prototypes_node->LinkEndChild((*it).writePrototypeXmlElement());
    }
    TiXmlElement *results_node = new TiXmlElement("Results");
    root_node->LinkEndChild(results_node);
    for(int i=0;i<proj->results.size();i++){
        TiXmlElement *result_node = new TiXmlElement("ResultFile");
        results_node->LinkEndChild(result_node);
        TiXmlText *result_text = new TiXmlText(proj->results[i].c_str());
        result_node->LinkEndChild(result_text);
    }
    TiXmlElement *properties_node = new TiXmlElement("Properties");
    root_node->LinkEndChild(properties_node);
    for(int i=0;i<proj->propertyNames.size();i++){
        TiXmlElement *property_node = new TiXmlElement("PropertyName");
        properties_node->LinkEndChild(property_node);
        TiXmlText *property_text = new TiXmlText(proj->propertyNames[i].c_str());
        property_node->LinkEndChild(property_text);
    }
    doc->SaveFile(filename.c_str());
    projectSaved = true;
}

void MainWindow::onProjectOpen(){
    if(!saveWarning())
        return;
    QString projectFile = QFileDialog::getOpenFileName(this,
                                                       tr("Open SoLIM Project"),
                                                       "./",
                                                       tr("Project file(*.slp)"));
    if(projectFile.isEmpty()){
        return;
    }
    TiXmlDocument doc(projectFile.toStdString().c_str());
    bool loadOK = doc.LoadFile();
    if (!loadOK) {
        throw invalid_argument("Failed to read xml file");
    }
    TiXmlHandle docHandle(&doc);
    TiXmlHandle projectHandle = docHandle.FirstChildElement("Project");
    proj = new SoLIMProject();
    proj->projName = projectHandle.ToElement()->Attribute("Name");
    proj->projFilename = projectHandle.FirstChildElement("ProjectFilename").ToElement()->GetText();

    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setData(model->index(0,0), proj->projName.c_str());

    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(0,0,prototypeChild);
    resultChild = new QStandardItem("Results");
    model->item(0,0)->setChild(1,0,resultChild);

    model->setHorizontalHeaderItem( 0, new QStandardItem("Projects") );

    //initialProjectView();
    projectView->setModel(model);
    projectView->expand(model->item(0,0)->index());
//    ui->dataTreeView->setModel( model );
//    ui->dataTreeView->expand(model->item(0,0)->index());

    TiXmlHandle gisDataHandle = projectHandle.FirstChildElement("GISData");
    for(TiXmlElement* layer = gisDataHandle.FirstChildElement("Layer").ToElement();
        layer; layer = layer->NextSiblingElement("Layer")){
        proj->layernames.push_back(layer->Attribute("Name"));
        proj->layertypes.push_back(layer->Attribute("Type"));
        proj->filenames.push_back(layer->GetText());
    }
    // add prototypes
    string tmpBaseName;
    TiXmlHandle prototypesHandle = projectHandle.FirstChildElement("Prototypes");
    for(TiXmlElement* prototype = prototypesHandle.FirstChildElement("Prototype").ToElement();
        prototype; prototype = prototype->NextSiblingElement()){
        Prototype proto;
        string basename = prototype->Attribute("BaseName");
        proto.prototypeBaseName = basename;
        if(tmpBaseName!=basename)
            proj->prototypeBaseNames.push_back(basename);
        tmpBaseName=basename;
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
    // set results
    TiXmlHandle resultHandle = projectHandle.FirstChildElement("Results");
    for(TiXmlElement* result = resultHandle.FirstChildElement("ResultFile").ToElement();
        result; result = result->NextSiblingElement("ResultFile")){
        proj->results.push_back(result->GetText());
    }
    onInferResults();
    TiXmlHandle propoertyHandle = projectHandle.FirstChildElement("Properties");
    for(TiXmlElement* property = propoertyHandle.FirstChildElement("PropertyName").ToElement();
        property; property = property->NextSiblingElement("PropertyName")){
        proj->propertyNames.push_back(property->GetText());
    }
    onGetPrototype();
    projectSaved = true;
    connect(projectView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)));
    ui->actionAdd_prototypes_from_samples->setEnabled(true);
}
void MainWindow::onSoilInferenceFromPrototypes(){
    if(proj){
        inference *infer= new inference(proj);
        infer->show();
        connect(infer,SIGNAL(finished(int)),this,SLOT(onInferResults()));
    }
}

void MainWindow::onAddPrototypeFromSamples(){
    getPrototype = new prototypeFromSamples(proj,this);
    getPrototype->show();
    // show prototypes
    connect(getPrototype,SIGNAL(finished(int)),this,SLOT(onGetPrototype()));
    projectSaved = false;
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
                    if((*it).envConditions[j].covariateName==proj->layernames[k].c_str()){
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

void MainWindow::onInferResults(){
    if(!proj)
        return;
    if(resultChild->rowCount()==proj->results.size()){
        return;
    }
    resultChild->setColumnCount(1);
    resultChild->setRowCount(0);
    for(int i = 0; i<proj->results.size();i++){
        resultChild->setRowCount(resultChild->rowCount()+1);
        resultChild->setChild(resultChild->rowCount()-1,0,new QStandardItem(proj->results[i].c_str()));
    }
    projectView->expand(resultChild->index());
    projectSaved = false;
}

void MainWindow::onViewData(){
    string filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open Raster File"),
                                                   "./",
                                                   tr("Raster file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)")).toStdString();
    if(!filename.empty())
        drawLayer(filename);
}

void MainWindow::onCustomContextMenu(const QPoint & point){
    QModelIndex index = projectView->indexAt(point);
    if(index.isValid()&&index.data().toString().compare("Prototypes")==0){
        prototypeMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    if(index.isValid()&&index.parent().data().toString().compare("Covariates")==0){
        viewDataMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    if(index.isValid()&&index.parent().data().toString().compare("Results")==0){
        viewDataMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
}

void MainWindow::drawLayer(string filename){
    myGraphicsView->showImage = true;
    zoomToolBar->setVisible(true);
    imgFilename = filename;
    myGraphicsView->getScene()->clear();
    string imagename = filename+".png";
    img = new QImage(imagename.c_str());
    BaseIO *lyr = new BaseIO(filename);
    imgMax = lyr->getDataMax();
    imgMin = lyr->getDataMin();
    if(img->isNull()){
        delete img;
        float* pafScanline = new float[lyr->getXSize()*lyr->getYSize()];
        unsigned char*imgData = new unsigned char[lyr->getXSize()*lyr->getYSize()];
        lyr->read(0,0,lyr->getYSize(),lyr->getXSize(),pafScanline);
        float range = 254.0/(imgMax-imgMin);
        for(int i = 0; i<lyr->getXSize()*lyr->getYSize();i++){
            float value = pafScanline[i];
            if(fabs(value-NODATA)<VERY_SMALL||value<NODATA){
                imgData[i]=255;
            }else{
                imgData[i] = (value-imgMin)*range;
            }
        }
        img = new QImage(imgData, lyr->getXSize(),lyr->getYSize(),lyr->getXSize(), QImage::Format_Grayscale8);
        img->save(imagename.c_str());
        if(pafScanline)
            delete []pafScanline;
    }
    delete lyr;
    int viewHeight = myGraphicsView->height();
    int viewWidth = myGraphicsView->width();
    myGraphicsView->getScene()->setSceneRect(0,0,viewWidth+30,viewHeight);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(*img).scaled(viewWidth,viewHeight,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    myGraphicsView->getScene()->addItem(item);
    item->setPos(30,0);
    QLinearGradient linear(QPoint(5,15),QPoint(5,65));
    linear.setColorAt(0,Qt::white);
    linear.setColorAt(1,Qt::black);
    linear.setSpread(QGradient::PadSpread);
    myGraphicsView->getScene()->addRect(5,15,10,50,QPen(QColor(255,255,255),0),linear);
    //std::ostringstream maxss;
    QGraphicsTextItem *minLabel = myGraphicsView->getScene()->addText(QString::number(imgMin));
    minLabel->setPos(15,50);
    QGraphicsTextItem *maxLabel = myGraphicsView->getScene()->addText(QString::number(imgMax));
    maxLabel->setPos(15,0);
    myGraphicsView->img = img;
    myGraphicsView->imgMax = imgMax;
    myGraphicsView->imgMin = imgMin;
    myGraphicsView->range = (imgMax-imgMin)/254.0;

}
void MainWindow::drawMembershipFunction(string basename, string idname, string covName){
    myGraphicsView->showImage = false;
    myGraphicsView->dataDetailsView->setModel(new QStandardItemModel(myGraphicsView->dataDetailsView));
    myGraphicsView->getScene()->clear();
    int protoPos = -1;
    int covPos = -1;
    for(int i = 0;i<proj->prototypes.size();i++){
        if(proj->prototypes[i].prototypeBaseName==basename
                && proj->prototypes[i].prototypeID==idname){
            protoPos = i;
            for(int j = 0; j<proj->prototypes[i].envConditionSize;j++){
                if(proj->prototypes[i].envConditions[j].covariateName==covName){
                    covPos = j;
                    break;
                }
            }
            break;
        }
    }
    if(protoPos==-1||covPos==-1){
        return;
    }
    myGraphicsView->getScene()->setSceneRect(0,0,myGraphicsView->width()*0.9,myGraphicsView->height()*0.9);
    QPen curvePen(Qt::black);
    QPen axisPen(Qt::blue);
    axisPen.setWidth(2);
    curvePen.setWidth(1);
    string coords = proj->prototypes[protoPos].envConditions[covPos].getCoords();
    vector<string> xycoord;
    solim::ParseStr(coords, ',', xycoord);
    int iKnotNum = xycoord.size();
    double x1,x2,y1,y2;
    vector<string> startCoord;
    vector<string> endCoord;
    solim::ParseStr(xycoord[0], ' ', startCoord);
    solim::ParseStr(xycoord[iKnotNum-1], ' ', endCoord);
    x1 = atof(startCoord[0].c_str());
    y1 = atof(startCoord[1].c_str());
    x2 = atof(endCoord[0].c_str());
    y2 = atof(endCoord[1].c_str());
    int sceneWidth = myGraphicsView->getScene()->width();
    int sceneHeight = myGraphicsView->getScene()->height();
    double max = (6*x2+x1)/7.0;
    double min = (6*x1+x2)/7.0;
    double scale = (fabs(max)>fabs(min))?fabs(max):fabs(min);
    if(scale<10) scale = int(scale)+1;
    else scale = (int(scale/10)+1)*10*2;
    int margin = 0.5*scale;
    // add info
    QGraphicsTextItem *covNameText = myGraphicsView->getScene()->addText(string("Covariate: "+covName).c_str());
    covNameText->setFont(QFont("Times", 10));
    covNameText->setPos(0.02*sceneWidth, 0.02*sceneHeight);
    string source = solim::PrototypeSource_str[proj->prototypes[protoPos].envConditions[covPos].source];
    QGraphicsTextItem *covSourceText = myGraphicsView->getScene()->addText(string("Source: "+source).c_str());
    covSourceText->setFont(QFont("Times", 10));
    covSourceText->setPos(0.02*sceneWidth, 0.02*sceneHeight+20);
    string typicalValue = to_string(proj->prototypes[protoPos].envConditions[covPos].typicalValue);
    QGraphicsTextItem *typicalValueText=myGraphicsView->getScene()->addText(string("Typical value: "+typicalValue).c_str());
    typicalValueText->setFont(QFont("Times", 10));
    typicalValueText->setPos(0.02*sceneWidth, 0.02*sceneHeight+40);

    // set axis
    myGraphicsView->getScene()->addLine(0.05*sceneWidth,0.85*sceneHeight,0.85*sceneWidth,0.85*sceneHeight,axisPen);
    myGraphicsView->getScene()->addLine(0.85*sceneWidth,0.85*sceneHeight+1,0.85*sceneWidth-3,0.85*sceneHeight+4,axisPen);
    myGraphicsView->getScene()->addLine(0.85*sceneWidth,0.85*sceneHeight,0.85*sceneWidth-3,0.85*sceneHeight-3,axisPen);
    myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.85*sceneHeight,0.45*sceneWidth,0.1*sceneHeight,axisPen);
    myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth-3,0.1*sceneHeight+3,axisPen);
    myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth+3,0.1*sceneHeight+3,axisPen);
    // set label
    myGraphicsView->getScene()->addLine(0.80*sceneWidth,0.85*sceneHeight,0.80*sceneWidth,0.85*sceneHeight-3,axisPen);
    myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.85*sceneHeight-3,axisPen);
    myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.15*sceneHeight,0.45*sceneWidth+3,0.15*sceneHeight,axisPen);

    // set axis names
    QGraphicsTextItem *yaxisName = myGraphicsView->getScene()->addText("Optimality value");
    yaxisName->setFont(QFont("Times", 10, QFont::Bold));
    yaxisName->setPos(0.45*sceneWidth, 0.1*sceneHeight-20);
    QGraphicsTextItem *xaxisName = myGraphicsView->getScene()->addText("Covariate value");
    xaxisName->setFont(QFont("Times", 10, QFont::Bold));
    xaxisName->setPos(0.85*sceneWidth, 0.85*sceneHeight-10);

    QGraphicsTextItem *yaxis1 = myGraphicsView->getScene()->addText("1");
    yaxis1->setFont(QFont("Times", 10, QFont::Bold));
    yaxis1->setPos(0.45*sceneWidth-15,0.15*sceneHeight-10);
    QGraphicsTextItem *yaxis0 = myGraphicsView->getScene()->addText("0");
    yaxis0->setFont(QFont("Times", 10, QFont::Bold));
    yaxis0->setPos(0.45*sceneWidth-5,0.85*sceneHeight);
    QGraphicsTextItem *xaxis1 = myGraphicsView->getScene()->addText(QString::number(margin));
    xaxis1->setFont(QFont("Times", 10, QFont::Bold));
    xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
    QGraphicsTextItem *xaxis0 = myGraphicsView->getScene()->addText(QString::number(-margin));
    xaxis0->setFont(QFont("Times", 10, QFont::Bold));
    xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
    double previousx,previousy;
    if(-margin>x1){
        previousy = proj->prototypes[protoPos].envConditions[covPos].getOptimality(0-margin);
        previousx = -margin;
    } else {
        previousy = proj->prototypes[protoPos].envConditions[covPos].getOptimality(min);
        previousx = min;
    }
    double x,y;
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;
    for(int i =0;i<100;i++){
        x =i*(max-min)/101.0+min;
        y = proj->prototypes[protoPos].envConditions[covPos].getOptimality(x);
        if(fabs(y+1)<VERY_SMALL ||fabs(previousy+1)<VERY_SMALL)
            continue;
        myGraphicsView->getScene()->addLine((previousx+margin)/scale*graphWidth+xStart,yEnd-previousy*graphHeight,(x+margin)/scale*graphWidth+xStart,yEnd-y*graphHeight,curvePen);
        previousx = x;
        previousy = y;
    }
    //myGraphicsView->getScene()->addLine((previousx-min)/scale*graphWidth+xStart,yEnd-previousy*graphHeight,xStart+graphWidth,yEnd-y2*graphHeight,curvePen);
}

void MainWindow::onZoomin()
{
    if(!img)
        return;
    int viewWidth = myGraphicsView->getScene()->width()-30;
    int viewHeight = myGraphicsView->getScene()->height();
    if(!(viewWidth<2*img->width()||viewHeight<2*img->height()))
        return;
    myGraphicsView->getScene()->clear();
    int height = viewHeight;
    int width = viewWidth;
    if(viewWidth<2*img->width()){
        width+=0.25*myGraphicsView->width();
    }
    if(viewHeight<2*img->height()){
        height+=0.25*myGraphicsView->height();
    }
    myGraphicsView->getScene()->setSceneRect(0,0,width+30,height);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(*img).scaled(width,height,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    item->setPos(30,0);
    myGraphicsView->getScene()->addItem(item);
    QLinearGradient linear(QPoint(5,15),QPoint(5,65));
    linear.setColorAt(0,Qt::white);
    linear.setColorAt(1,Qt::black);
    linear.setSpread(QGradient::PadSpread);
    myGraphicsView->getScene()->addRect(5,15,10,50,QPen(QColor(255,255,255),0),linear);
    //std::ostringstream maxss;
    QGraphicsTextItem *minLabel = myGraphicsView->getScene()->addText(QString::number(imgMin));
    minLabel->setPos(15,50);
    QGraphicsTextItem *maxLabel = myGraphicsView->getScene()->addText(QString::number(imgMax));
    maxLabel->setPos(15,0);
}

void MainWindow::onZoomout()
{
    if(!img)
        return;
    int viewWidth = myGraphicsView->getScene()->width()-30;
    int viewHeight =myGraphicsView->getScene()->height();
    if(!(viewWidth>myGraphicsView->width()/4||viewHeight>myGraphicsView->height()/4))
        return;
    myGraphicsView->getScene()->clear();
    int height = viewHeight;
    int width = viewWidth;
    if(viewWidth>myGraphicsView->width()/4){
        width-=0.25*myGraphicsView->width();
    }
    if(viewHeight>myGraphicsView->width()/4){
        height-=0.25*myGraphicsView->height();
    }
    myGraphicsView->getScene()->setSceneRect(0,0,width+30,height);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(*img).scaled(width,height,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    item->setPos(30,0);
    myGraphicsView->getScene()->addItem(item);
    QLinearGradient linear(QPoint(5,15),QPoint(5,65));
    linear.setColorAt(0,Qt::white);
    linear.setColorAt(1,Qt::black);
    linear.setSpread(QGradient::PadSpread);
    myGraphicsView->getScene()->addRect(5,15,10,50,QPen(QColor(255,255,255),0),linear);
    //std::ostringstream maxss;
    QGraphicsTextItem *minLabel = myGraphicsView->getScene()->addText(QString::number(imgMin));
    minLabel->setPos(15,50);
    QGraphicsTextItem *maxLabel = myGraphicsView->getScene()->addText(QString::number(imgMax));
    maxLabel->setPos(15,0);
}
