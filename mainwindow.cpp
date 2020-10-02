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
    connect(ui->actionSave_as,SIGNAL(triggered()),this,SLOT(onProjectSaveAs()));
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
    resultChild = nullptr;
    prototypeChild = nullptr;
    gisDataChild = nullptr;
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
    if(resultChild!=nullptr) delete resultChild;
    if(prototypeChild!=nullptr)  delete prototypeChild;
    if(proj!=nullptr)    delete proj;
}

//======================================= Main menu ================================================
void MainWindow::onProjectNew(){
    if(!saveWarning())
        return;
    NewProjectDialog newProject;
    newProject.exec();
    QString projName = newProject.projectName;
    QString studyArea = newProject.studyArea;
    if(projName.isEmpty()) return;
    proj = new SoLIMProject();
    proj->projFilename = newProject.projectFilename.toStdString();
    proj->projName = projName.toStdString();
    proj->studyArea=studyArea.toStdString();
    initModel();

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
    TiXmlElement *studyArea_node = new TiXmlElement("StudyArea");
    root_node->LinkEndChild(studyArea_node);
    TiXmlText *studyarea_text = new TiXmlText(proj->studyArea.c_str());
    studyArea_node->LinkEndChild(studyarea_text);
    TiXmlElement *gisData_node = new TiXmlElement("GISData");
    root_node->LinkEndChild(gisData_node);
    TiXmlElement *prototypes_node = new TiXmlElement("Prototypes");
    root_node->LinkEndChild(prototypes_node);
    updateGisDataFromTree();
    for (int i = 0; i<proj->filenames.size();i++) {
        // add layer to GISData
        TiXmlElement *layer_node = new TiXmlElement("Layer");
        layer_node->SetAttribute("Name",proj->layernames[i].c_str());
        layer_node->SetAttribute("Type",proj->layertypes[i].c_str());
        gisData_node->LinkEndChild(layer_node);

        TiXmlText *layer_text = new TiXmlText(proj->filenames[i].c_str());
        layer_node->LinkEndChild(layer_text);
    }
    for(int i =0; i<proj->prototypeBaseNames.size();i++){
        TiXmlElement *prototypeBase_node = new TiXmlElement("PrototypeBase");
            prototypeBase_node->SetAttribute("Basename",proj->prototypeBaseNames[i].c_str());
        prototypes_node->LinkEndChild(prototypeBase_node);
        for(vector<solim::Prototype>::iterator it = proj->prototypes.begin();it!=proj->prototypes.end();it++){
            if((*it).prototypeBaseName==proj->prototypeBaseNames[i])
                prototypeBase_node->LinkEndChild((*it).writePrototypeXmlElement());
        }
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

void MainWindow::onProjectSaveAs(){
    if(!proj) return;
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save project as"),
                                                    tr("*.slp"),
                                                    "./");
    proj->projFilename=filename.toStdString();
    onProjectSave();
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
    if(projectHandle.FirstChildElement("StudyArea").ToElement()->GetText())
        proj->studyArea = projectHandle.FirstChildElement("StudyArea").ToElement()->GetText();
    else
        proj->studyArea = "";

    initModel();
    // read gis data
    TiXmlHandle gisDataHandle = projectHandle.FirstChildElement("GISData");
    for(TiXmlElement* layer = gisDataHandle.FirstChildElement("Layer").ToElement();
        layer; layer = layer->NextSiblingElement("Layer")){
        proj->layernames.push_back(layer->Attribute("Name"));
        proj->layertypes.push_back(layer->Attribute("Type"));
        proj->filenames.push_back(layer->GetText());
    }
    onGetGisData();
    // add prototypes
    string tmpBaseName;
    TiXmlHandle prototypesHandle = projectHandle.FirstChildElement("Prototypes");

    for(TiXmlElement* prototypeBase = prototypesHandle.FirstChildElement("PrototypeBase").ToElement();
        prototypeBase; prototypeBase = prototypeBase->NextSiblingElement()){
        proj->prototypeBaseNames.push_back(prototypeBase->Attribute("Basename"));
        readPrototype(prototypeBase);
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

}

void MainWindow::onSoilInferenceFromPrototypes(){
    updateGisDataFromTree();
    if(proj){
        inference *infer= new inference(proj);
        infer->show();
        connect(infer,SIGNAL(finished(int)),this,SLOT(onInferResults()));
    }
}

void MainWindow::onViewData(){
    string filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open Raster File"),
                                                   "./",
                                                   tr("Raster file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)")).toStdString();
    if(!filename.empty())
        drawLayer(filename);
}

//==================================== Project tree slots ==========================================
void MainWindow::onSelectionChanged(const QItemSelection& current,const QItemSelection& previous){
    QModelIndex index = current.indexes().at(0);
    if(index.isValid()&&index.parent().data().toString().compare("Results")==0){
        string filename = index.data().toString().toStdString();
        drawLayer(filename);
    }
    else if(index.isValid()&&index.parent().data().toString().compare("Covariates")==0){
        string layername=index.data().toString().midRef(11).toString().toStdString();
        string filename="";
        for(int k = 0;k < proj->layernames.size();k++){
            if(layername==proj->layernames[k].c_str()){
                filename=proj->filenames[k];
                break;
            }
        }
        drawLayer(filename);
    }
    else if(index.isValid()&&index.parent().data().toString().compare("GIS Data")==0){
        string filename = index.child(0,0).data().toString().midRef(10).toString().toStdString();
        drawLayer(filename);
    }
    else if(index.isValid()&&index.data().toString().compare("Membership Function")==0){
        string prototype = index.parent().parent().parent().data().toString().toStdString();
        int prefixLength = 11;
        int idPrefixLength = 14;
        int basePrefixLength = 16;
        string baseName = index.parent().parent().parent().parent().data().toString().toStdString().substr(basePrefixLength);
        string protoID = prototype.substr(idPrefixLength);
        string covName = index.parent().data().toString().toStdString().substr(prefixLength);
        drawMembershipFunction(baseName,protoID,covName);
    }
}

void MainWindow::onCustomContextMenu(const QPoint & point){
    QModelIndex index = projectView->indexAt(point);
    if(index.isValid()&&index.data().toString().compare("Prototypes")==0){
        prototypeMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    else if(index.isValid()&&index.data().toString().compare("GIS Data")==0){
        gisDataMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    if(index.isValid()&&index.parent().data().toString().compare("Prototypes")==0){
        currentBaseName=index.data().toString().mid(16).toStdString();
        prototypeBaseMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
}

void MainWindow::onAddGisData(){
    AddGisDataDialog addGisData(this);
    addGisData.exec();
    if(addGisData.filename.isEmpty()){
        return;
    }
    for(int i = 0;i<proj->filenames.size();i++){
        if(proj->filenames[i]==addGisData.filename.toStdString()){
            QMessageBox warning;
            warning.setText("This file already exists in GIS data.");
            warning.exec();
            return;
        }
        if(proj->layernames[i]==addGisData.covariate.toStdString()){
            QMessageBox warning;
            warning.setText("This covariate already exists in GIS data. Please rename the covariate.");
            warning.exec();
            return;
        }
    }
    proj->filenames.push_back(addGisData.filename.toStdString());
    proj->layernames.push_back(addGisData.covariate.toStdString());
    proj->layertypes.push_back(addGisData.datatype);
    onGetGisData();
}

void MainWindow::onAddPrototypeFromSamples(){
    getPrototype = new prototypeFromSamples(proj,this);
    getPrototype->exec();
    // show prototypes
    //connect(getPrototype,SIGNAL(finished(int)),this,SLOT(onGetPrototype()));
    onGetPrototype();
    projectSaved = false;
}

void MainWindow::onAddPrototypeFromExpert(){
    qInfo()<<"add expert";
}

void MainWindow::onAddPrototypeFromMining(){
    qInfo()<<"add maps";
}

void MainWindow::onImportPrototypeBase(){
    QString basefilename=QFileDialog::getOpenFileName(this,tr("Open prototype base file"),"./",tr("(*.csv *.xml)"));
    if(basefilename.isEmpty())  return;
    if(basefilename.endsWith(".csv",Qt::CaseInsensitive)){
        QFile basefile(basefilename);
        if(!basefile.open(QFile::ReadOnly)){
            QMessageBox warning;
            warning.setText("File open failed!");
            warning.exec();
            return;
        }
        QTextStream *out = new QTextStream(&basefile);
        QStringList lines = out->readAll().split("\n");
        if(lines.size()<1)  return;
        QStringList basenames = lines[0].split(",");
        if(basenames[0]!="basename"){ wrongFormatWarning(); return; }
        string basename = basenames[1].toStdString();
        if(baseExistsWarning(basename)) return;
        proj->prototypeBaseNames.push_back(basename);
        if(basenames[2]!="source"){ wrongFormatWarning(); return; }
        string source = basenames[3].toStdString();
        solim::PrototypeSource protoSource = solim::getSourceFromString(source);

        for(int i =1;i<lines.size();i++){
            if(lines[i].isEmpty()) break;
            QStringList properties = lines[i].split(",");
            if(properties[0]!="prototype"){ wrongFormatWarning(); return; }
            Prototype proto;
            proto.prototypeBaseName=basename;
            proto.prototypeID = properties[1].toStdString();
            proto.source = protoSource;
            if(properties[2]!="properties_num"){ wrongFormatWarning(); return; }
            int properties_num = properties[3].toInt();
            int loc = 4;
            for(int k=0;k<properties_num;k++){
                proto.addProperties(properties[loc].toStdString(),properties[loc+1].toDouble());
                loc += 2;
                if(i==1)
                    proj->propertyNames.push_back(properties[loc].toStdString());
            }
            if(properties[loc]!="covariates_num"){ wrongFormatWarning(); return; }
            int cov_num = properties[loc+1].toInt();
            loc+=2;
            for(int k = 0;k<cov_num;k++){
                if(properties[loc]!="covariate_name"){ wrongFormatWarning(); return; }
                string covname = properties[loc+1].toStdString();
                if(properties[loc+2]!="datatype"){ wrongFormatWarning(); return; }
                solim::DataTypeEnum datatype = solim::getDatatypeFromString(properties[loc+3].toStdString());
                if(properties[loc+6]!="range"){ wrongFormatWarning(); return; }
                int range = properties[loc+7].toInt();
                if(properties[loc+8]!="Node_num"){ wrongFormatWarning(); return; }
                int node_num = properties[loc+9].toDouble();
                if(properties[loc+10]!="Membership_function"){ wrongFormatWarning(); return; }
                loc+=11;
                QStringList coords;
                for(int n = 0;n<node_num;n++){
                    coords.push_back(properties[loc]);
                    ++loc;
                }
                string coords_str = coords.join(",").toStdString();
                solim::Curve *c = new solim::Curve(covname, datatype, node_num, coords_str, range);
                proto.envConditions.push_back(*c);
                proto.envConditionSize++;
            }
            proj->prototypes.push_back(proto);
        }
        onGetPrototype();
        projectSaved=false;
    } if(basefilename.endsWith(".xml",Qt::CaseInsensitive)){
        TiXmlDocument doc(basefilename.toStdString().c_str());
        bool loadOK = doc.LoadFile();
        if (!loadOK) {
            throw invalid_argument("Failed to read xml file");
        }
        TiXmlHandle docHandle(&doc);
        TiXmlHandle prototypeBaseHandle = docHandle.FirstChildElement("PrototypeBase");
        string basename=prototypeBaseHandle.ToElement()->Attribute("Basename");
        if(baseExistsWarning(basename)) return;
        proj->prototypeBaseNames.push_back(basename);
        readPrototype(prototypeBaseHandle.ToElement());
        onGetPrototype();
        projectSaved=false;
    }
}

void MainWindow::onChangeCovName(){
    int i = 0;
    while(i < proj->prototypes.size()){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName)
            break;
        i++;
    }
    class changeCovName changeName(&(proj->prototypes[i]),this);
    changeName.exec();
    if(changeName.isChanged){
        for(int i =0;i<proj->prototypes.size();i++){
            if(proj->prototypes[i].prototypeBaseName==currentBaseName){
                for(int j = 0;j<proj->prototypes[i].envConditionSize;j++){
                    proj->prototypes[i].envConditions[j].covariateName=changeName.proto->envConditions[j].covariateName;
                }
            }
        }
        onGetPrototype();
        projectSaved=false;
    }
}

void MainWindow::onSavePrototypeBase(){
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save prototype base as"),
                                                    ".",
                                                    tr("(*.xml)"));

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc->LinkEndChild(pDeclaration);
    TiXmlElement *prototypes_node = new TiXmlElement("PrototypeBase");
    prototypes_node->SetAttribute("Basename",currentBaseName.c_str());
    doc->LinkEndChild(prototypes_node);
    for(vector<solim::Prototype>::iterator it = proj->prototypes.begin();it!=proj->prototypes.end();it++){
        if((*it).prototypeBaseName==currentBaseName)
            prototypes_node->LinkEndChild((*it).writePrototypeXmlElement());
    }
    doc->SaveFile(filename.toStdString().c_str());
}

void MainWindow::onExportPrototypeBase(){
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Export prototype base as"),
                                                    ".",
                                                    tr("(*.csv)"));
    if(filename.isEmpty()){
        QMessageBox warning;
        warning.setText("Please input file name!");
        warning.exec();
        return;
    }
    QFile basefile(filename);
    if(!basefile.open(QFile::WriteOnly)){
        QMessageBox warning;
        warning.setText("File open failed!");
        warning.exec();
        return;
    }
    if(currentBaseName.find(',')!=std::string::npos){
        QMessageBox warning;
        warning.setText("Please do not use ',' in prototype base name");
        warning.exec();
        return;
    }
    QTextStream stream(&basefile);
    stream<<"basename,"<<currentBaseName.c_str()<<",source,"<<solim::PrototypeSource_str[proj->prototypes[0].source]<<"\n";
    for(int i = 0;i<proj->prototypes.size();i++){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName){
            solim::Prototype *proto = &(proj->prototypes[i]);
            stream<<"prototype,"<<proto->prototypeID.c_str()<<",properties_num,"<<QString::number(proto->properties.size())<<",";
            for(int j=0;j<proto->properties.size();j++){
                if(proto->properties[j].propertyName.find(',')!=std::string::npos){
                    QMessageBox warning;
                    warning.setText("Please do not use ',' in prototype property name");
                    warning.exec();
                    return;
                }
                stream<<proto->properties[j].propertyName.c_str()<<","<<QString::number(proto->properties[j].propertyValue)<<",";
            }
            stream<<"covariates_num,"<<QString::number(proto->envConditionSize)<<",";
            for(int j = 0;j<proto->envConditionSize;j++){
                stream<<"covariate_name,"<<proto->envConditions[j].covariateName.c_str()<<",";
                stream<<"datatype,"<<solim::DataTypeEnum_str[proto->envConditions[j].dataType]<<",";
                stream<<"typical_value,"<<QString::number(proto->envConditions[j].typicalValue)<<",";
                stream<<"range,"<<QString::number(proto->envConditions[j].range)<<",";
                stream<<"Node_num,"<<QString::number(proto->envConditions[j].getKnotNum())<<",";
                stream<<"Membership_function,"<<proto->envConditions[j].getCoords().c_str()<<",";
            }
            stream<<"\n";
        }
    }
    basefile.close();
    QMessageBox success;
    success.setText("Prototype base file saved!");
    success.exec();
    return;
}

//=================================== upadte project tree view ==================================
void MainWindow::onGetGisData(){
    gisDataChild->setColumnCount(1);
    if(proj->filenames.size()>gisDataChild->rowCount()){
        for(int i = 0;i<proj->filenames.size();i++){
            gisDataChild->setChild(i,0,new QStandardItem(proj->layernames[i].c_str()));
            gisDataChild->child(i)->setChild(0,0,new QStandardItem(("Filename: "+proj->filenames[i]).c_str()));
            gisDataChild->child(i)->setChild(1,0,new QStandardItem(("Type: "+proj->layertypes[i]).c_str()));
        }
    }
}

void MainWindow::onGetPrototype(){
    prototypeChild->setColumnCount(1);
    if(proj->prototypes.size()>0){
        for(int i =0;i<proj->prototypeBaseNames.size();i++){
            string prototypebase = proj->prototypeBaseNames[i];
            QStandardItem *prototypeBase = new QStandardItem(("Prototype Base: "+prototypebase).c_str());
            prototypeChild->setChild(i,0,prototypeBase);
            for(vector<Prototype>::iterator it = proj->prototypes.begin(); it!=proj->prototypes.end();it++){
                if((*it).prototypeBaseName==prototypebase){
                    QStandardItem* prototype = new QStandardItem(("Prototype ID: "+(*it).prototypeID).c_str());
                    prototypeBase->setChild(prototypeBase->rowCount(),0,prototype);
                    prototype->setColumnCount(1);
                    prototype->setChild(0,0,new QStandardItem(("Source: "+string(solim::PrototypeSource_str[(*it).source])).c_str()));
                    QStandardItem* properties = new QStandardItem("Properties");
                    prototype->setChild(1,0,properties);
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
                    prototype->setChild(2,0,covariates);
                    covariates->setColumnCount(1);
                    for(int j = 0; j<(*it).envConditionSize;j++){
                        string cov = "Covariate: " + (*it).envConditions[j].covariateName;
                        covariates->setRowCount(covariates->rowCount()+1);
                        QStandardItem *covItem = new QStandardItem(cov.c_str());
                        covariates->setChild(covariates->rowCount()-1,0,covItem);
                        covItem->setColumnCount(1);
                        double typicalValue = (*it).envConditions[j].typicalValue;
                        if(fabs(typicalValue-NODATA)>VERY_SMALL){
                            string typicalV = "Typical value: "+to_string(typicalValue);
                            covItem->setChild(covItem->rowCount(),0,new QStandardItem(typicalV.c_str()));
                        }
                        // set membership function
                        covItem->setChild(covItem->rowCount(),0,new QStandardItem("Membership Function"));
                    }
                }
            }
        }
    }
    onGetGisData();
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

//=========================== Main Graphics View function===============================
void MainWindow::drawLayer(string filename){
    if(filename.empty()) return;
    myGraphicsView->showImage = true;
    zoomToolBar->setVisible(true);
    imgFilename = filename;
    myGraphicsView->getScene()->clear();
    string imagename = filename+".png";
    img = new QImage(imagename.c_str());
    BaseIO *lyr = new BaseIO(filename);
    if(!lyr->openSuccess) return;
    double imgMax = lyr->getDataMax();
    double imgMin = lyr->getDataMin();
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
    double scale = proj->prototypes[protoPos].envConditions[covPos].range;
    if(scale<10) scale = int(scale)+1;
    else scale = (int(scale/10)+1)*10*2;
    int margin = 0.5*scale;
    // add info
    QGraphicsTextItem *covNameText = myGraphicsView->getScene()->addText(string("Covariate: "+covName).c_str());
    covNameText->setFont(QFont("Times", 10));
    covNameText->setPos(0.02*sceneWidth, 0.02*sceneHeight);
    string source = solim::PrototypeSource_str[proj->prototypes[protoPos].source];
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
    previousy = proj->prototypes[protoPos].envConditions[covPos].getOptimality(0-margin);
    previousx = -margin;

    double x,y;
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;
    for(int i =0;i<100;i++){
        x =i*2*margin/101.0-margin;
        y = proj->prototypes[protoPos].envConditions[covPos].getOptimality(x);
        if(fabs(y+1)<VERY_SMALL ||fabs(previousy+1)<VERY_SMALL)
            continue;
        myGraphicsView->getScene()->addLine((previousx+margin)/scale*graphWidth+xStart,yEnd-previousy*graphHeight,(x+margin)/scale*graphWidth+xStart,yEnd-y*graphHeight,curvePen);
        previousx = x;
        previousy = y;
    }
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
    QGraphicsTextItem *minLabel = myGraphicsView->getScene()->addText(QString::number(myGraphicsView->imgMin));
    minLabel->setPos(15,50);
    QGraphicsTextItem *maxLabel = myGraphicsView->getScene()->addText(QString::number(myGraphicsView->imgMax));
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
    QGraphicsTextItem *minLabel = myGraphicsView->getScene()->addText(QString::number(myGraphicsView->imgMin));
    minLabel->setPos(15,50);
    QGraphicsTextItem *maxLabel = myGraphicsView->getScene()->addText(QString::number(myGraphicsView->imgMax));
    maxLabel->setPos(15,0);
}

//=========================== non-slot functions ============================================
void MainWindow::updateGisDataFromTree(){
    proj->filenames.clear();
    proj->layernames.clear();
    proj->layertypes.clear();
    for (int i = 0; i<gisDataChild->rowCount();i++) {
        proj->layernames.push_back(gisDataChild->child(i,0)->text().toStdString());
        proj->filenames.push_back(gisDataChild->child(i,0)->child(0,0)->text().mid(10).toStdString());
        proj->layertypes.push_back(gisDataChild->child(i,0)->child(1,0)->text().mid(6).toStdString());
    }
}

void MainWindow::initDataDetailsView(){
    dataDetailsDock = new QDockWidget(tr("Data details"), this);
    dataDetailsView = new QTableView(dataDetailsDock);
    dataDetailsView->verticalHeader()->hide();
    dataDetailsView->horizontalHeader()->hide();
    dataDetailsView->setShowGrid(false);
    dataDetailsDock->setFeatures(dataDetailsDock->features() & ~QDockWidget::DockWidgetClosable);
    dataDetailsDock->setWidget(dataDetailsView);
    dataDetailsDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::BottomDockWidgetArea);
}

void MainWindow::initModel(){
    model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setData(model->index(0,0), proj->projName.c_str());
    if(!proj->studyArea.empty()){
        model->item(0,0)->setChild(0,0, new QStandardItem(("Study area: "+proj->studyArea).c_str()));
    }
    gisDataChild = new QStandardItem("GIS Data");
    model->item(0,0)->setChild(model->item(0,0)->rowCount(),0,gisDataChild);
    prototypeChild = new QStandardItem("Prototypes");
    model->item(0,0)->setChild(model->item(0,0)->rowCount(),0,prototypeChild);
    resultChild = new QStandardItem("Results");
    model->item(0,0)->setChild(model->item(0,0)->rowCount(),0,resultChild);
    model->setHorizontalHeaderItem( 0, new QStandardItem("Projects") );

    projectView->setModel(model);
    projectView->expand(model->item(0,0)->index());

    projectSaved = false;
    connect(projectView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)));
    ui->actionAdd_prototypes_from_samples->setEnabled(true);
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
    QAction *prototypesFromSamples = new QAction("Create new prototype base from samples",prototypeMenu);
    prototypeMenu->addAction(prototypesFromSamples);
    projectView->addAction(prototypesFromSamples);
    QAction *prototypesFromExpert = new QAction("Create new prototype base from expert",prototypeMenu);
    prototypeMenu->addAction(prototypesFromExpert);
    projectView->addAction(prototypesFromExpert);
    QAction *prototypesFromMining = new QAction("Create new prototype base from data mining",prototypeMenu);
    prototypeMenu->addAction(prototypesFromMining);
    projectView->addAction(prototypesFromMining);
    QAction *importPrototypeBase = new QAction("Import prototype base from file",prototypeMenu);
    prototypeMenu->addAction(importPrototypeBase);
    projectView->addAction(importPrototypeBase);
    connect(prototypesFromSamples,SIGNAL(triggered()),this,SLOT(onAddPrototypeFromSamples()));
    connect(prototypesFromExpert,SIGNAL(triggered()),this,SLOT(onAddPrototypeFromExpert()));
    connect(prototypesFromMining,SIGNAL(triggered()),this,SLOT(onAddPrototypeFromMining()));
    connect(importPrototypeBase,SIGNAL(triggered()),this,SLOT(onImportPrototypeBase()));
    gisDataMenu = new QMenu(projectView);
    QAction *addGisData = new QAction("Add GIS Data", gisDataMenu);
    gisDataMenu->addAction(addGisData);
    projectView->addAction(addGisData);
    connect(addGisData,SIGNAL(triggered()),this,SLOT(onAddGisData()));
    prototypeBaseMenu = new QMenu(projectView);
    QAction *changeCovName = new QAction("Change covariate name",prototypeBaseMenu);
    prototypeBaseMenu->addAction(changeCovName);
    projectView->addAction(changeCovName);
    QAction *savePrototypeBase_xml = new QAction("Save as external .xml file",prototypeBaseMenu);
    prototypeBaseMenu->addAction(savePrototypeBase_xml);
    projectView->addAction(savePrototypeBase_xml);
    QAction *exportPrototypeBase_csv = new QAction("Export to .csv file",prototypeBaseMenu);
    prototypeBaseMenu->addAction(exportPrototypeBase_csv);
    projectView->addAction(exportPrototypeBase_csv);
    connect(changeCovName,SIGNAL(triggered()),this,SLOT(onChangeCovName()));
    connect(exportPrototypeBase_csv,SIGNAL(triggered()),this,SLOT(onExportPrototypeBase()));
    connect(savePrototypeBase_xml, SIGNAL(triggered()),this,SLOT(onSavePrototypeBase()));
    //projectView->addAction(viewData);
    projectViewInitialized = true;
}

void MainWindow::readPrototype(TiXmlElement*prototypesElement){
    for(TiXmlElement* prototype = prototypesElement->FirstChildElement("Prototype");
        prototype; prototype = prototype->NextSiblingElement()){
        Prototype proto;
        string basename = prototype->Attribute("BaseName");
        string source = prototype->Attribute("Source");
        proto.prototypeBaseName = basename;
        proto.source=solim::getSourceFromString(source);
        proto.prototypeID = prototype->Attribute("ID");
        TiXmlElement* envConditons_node = prototype->FirstChildElement("CurveLib");
        for(TiXmlElement *envAttri = envConditons_node->FirstChildElement("EnvAttri");
            envAttri; envAttri = envAttri->NextSiblingElement()){
            TiXmlElement *curveElement = envAttri->FirstChildElement("Curve");
            string covName = envAttri->Attribute("Name");
            solim::DataTypeEnum datatype = solim::getDatatypeFromString(curveElement->FirstChildElement("DataType")->GetText());
            int nodeNum = atoi(curveElement->FirstChildElement("NodeNum")->GetText());
            string coords = curveElement->FirstChildElement("Coordinates")->GetText();
            double range = atof(curveElement->FirstChildElement("Range")->GetText());
            solim::Curve *c = new solim::Curve(covName, datatype, nodeNum, coords, range);
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

void MainWindow::wrongFormatWarning(){
    QMessageBox warning;
    warning.setText("Wrong file format");
    warning.exec();
    return;
}

bool MainWindow::baseExistsWarning(string basename){
    for(int i = 0; i< proj->prototypeBaseNames.size();i++){
        if(basename==proj->prototypeBaseNames[i]){
            QMessageBox warning;
            warning.setText("Prototype base with the same base name already exists.");
            warning.exec();
            return true;
        }
    }
    return false;
}
