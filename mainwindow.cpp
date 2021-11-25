#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->actionAdd_prototypes_from_samples->setDisabled(true);
    //ui->menuSample_Design->setDisabled(true);
    ui->actionAdd_prototypes_from_Data_Mining->setDisabled(true);
    ui->actionAdd_prototypes_from_expert->setDisabled(true);
    ui->actionSave->setDisabled(true);
    ui->actionSave_as->setDisabled(true);
    ui->actionClose_Project->setDisabled(true);
    ui->actionDefine_Study_Area->setDisabled(true);
    ui->action_infer->setDisabled(true);
    setWindowIcon(QIcon("./imgs/solim.jpg"));
    // setup dock view
    projectViewInitialized = false;
    initialProjectView();
    addDockWidget(Qt::LeftDockWidgetArea,projectDock);

    // setup data details dock
    initDataDetailsView();
    addDockWidget(Qt::LeftDockWidgetArea,dataDetailsDock);
    // setup main menu
    connect(&createImgThread, SIGNAL(finished()), this, SLOT(finishedCreateImg())); //cant have parameter sorry, when using connect
    connect(&createImgThread, SIGNAL(started()), this, SLOT(createImg())); //cant have parameter sorry, when using connect
    projectSaved = true;
    img = nullptr;
    proj = nullptr;
    myGraphicsView = new MyGraphicsView();
    ui->centralwidget->layout()->addWidget(myGraphicsView);
    myGraphicsView->dataDetailsView = dataDetailsView;
    connect(myGraphicsView,SIGNAL(addFreehandPoint()),this,SLOT(onAddFreehandPoint()));
    connect(myGraphicsView,SIGNAL(addEnumPoint()),this,SLOT(onAddEnumPoint()));
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
    resetRangeToolBar = addToolBar(tr("reset membership function range"));
    QAction *resetRangeAct = new QAction("reset range", this);
    connect(resetRangeAct, SIGNAL(triggered()),this,SLOT(onResetRange()));
    resetRangeToolBar->addAction(resetRangeAct);
    resetRangeToolBar->setVisible(false);
    resultChild = nullptr;
    prototypeChild = nullptr;
    gisDataChild = nullptr;
    addRule = nullptr;
    graphicFilename="";
    initParas();
}

MainWindow::~MainWindow()
{
    if(!projectSaved){
        saveWarning();
    }
    saveSetting();
    delete ui;
    if(img!=nullptr) delete img;
    if(projectView!=nullptr) delete projectView;
    if(projectDock!=nullptr) delete projectDock;
    if(resultChild!=nullptr) delete resultChild;
    if(prototypeChild!=nullptr)  delete prototypeChild;
    if(proj!=nullptr)    delete proj;
}

//======================================= Main menu ================================================
void MainWindow::on_actionNew_triggered(){
    // project new
    if(!saveWarning())
        return;
    NewProjectDialog newProject(workingDir,this);
    newProject.exec();
    QString projName = newProject.projectName;
    QString studyArea = newProject.studyArea;
    if(projName.isEmpty()) return;
    proj = new SoLIMProject();
    proj->projFilename = newProject.projectFilename.toStdString();
    proj->projName = projName.toStdString();
    proj->studyArea=studyArea.toStdString();
    initModel();
    workingDir=workingDir=QFileInfo(newProject.projectFilename).absoluteDir().absolutePath();
    proj->workingDir=workingDir;
}

void MainWindow::on_actionSave_triggered(){
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
    TiXmlElement *studyArea_node = new TiXmlElement("StudyArea");
    root_node->LinkEndChild(studyArea_node);
    TiXmlText *studyarea_text = new TiXmlText(proj->studyArea.c_str());
    studyArea_node->LinkEndChild(studyarea_text);
    TiXmlElement *gisData_node = new TiXmlElement("GISData");
    root_node->LinkEndChild(gisData_node);
    TiXmlElement *prototypes_node = new TiXmlElement("Prototypes");
    root_node->LinkEndChild(prototypes_node);
    for (size_t i = 0; i<proj->filenames.size();i++) {
        // add layer to GISData
        TiXmlElement *layer_node = new TiXmlElement("Layer");
        layer_node->SetAttribute("Name",proj->layernames[i].c_str());
        layer_node->SetAttribute("Type",proj->layertypes[i].c_str());
        gisData_node->LinkEndChild(layer_node);

        TiXmlText *layer_text = new TiXmlText(proj->filenames[i].c_str());
        layer_node->LinkEndChild(layer_text);
    }
    for(size_t i =0; i<proj->prototypeBaseNames.size();i++){
        TiXmlElement *prototypeBase_node = new TiXmlElement("PrototypeBase");
            prototypeBase_node->SetAttribute("Basename",proj->prototypeBaseNames[i].c_str());
            prototypeBase_node->SetAttribute("Source", proj->prototypeBaseTypes[i].c_str());
        prototypes_node->LinkEndChild(prototypeBase_node);
        for(vector<solim::Prototype>::iterator it = proj->prototypes.begin();it!=proj->prototypes.end();it++){
            if((*it).prototypeBaseName==proj->prototypeBaseNames[i])
                prototypeBase_node->LinkEndChild((*it).writePrototypeXmlElement());
        }
    }
    TiXmlElement *results_node = new TiXmlElement("Results");
    root_node->LinkEndChild(results_node);
    for(size_t i=0;i<proj->results.size();i++){
        TiXmlElement *result_node = new TiXmlElement("ResultFile");
        results_node->LinkEndChild(result_node);
        TiXmlText *result_text = new TiXmlText(proj->results[i].c_str());
        result_node->LinkEndChild(result_text);
    }
    if(doc->SaveFile(filename.c_str()))
        projectSaved = true;
    else {
        QMessageBox warn;
        warn.setText("Project file save failed!");
        warn.exec();
    }
}

void MainWindow::on_actionSave_as_triggered(){
    if(!proj) return;
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save project as"),
                                                    workingDir,
                                                    tr("*.slp"));
    proj->projFilename=filename.toStdString();
    if(!filename.isEmpty()){
        std::size_t first = filename.lastIndexOf('/');
        if (first==std::string::npos){
            first = filename.lastIndexOf('\\');
        }
        std::size_t end = filename.lastIndexOf('.');
        proj->projName = filename.mid(first+1,end-first-1).toStdString();
    }
    workingDir=QFileInfo(filename).absoluteDir().absolutePath();
    model->setData(model->index(0,0), proj->projName.c_str());
    on_actionSave_triggered();
}

void MainWindow::on_actionOpen_triggered(){
    // project open
    if(!saveWarning())
        return;
    QString projectFile = QFileDialog::getOpenFileName(this,
                                                       tr("Open SoLIM Project"),
                                                       workingDir,
                                                       tr("Project file(*.slp)"));
    if(projectFile.isEmpty()){
        return;
    }
   workingDir=QFileInfo(projectFile).absoluteDir().absolutePath();
   TiXmlDocument doc(projectFile.toStdString().c_str());
    bool loadOK = doc.LoadFile();
    if (!loadOK) {
        QMessageBox msg;
        msg.setText("Failed to open project file. Please make sure the filename and path to file does not contain non-English characters.");
        msg.exec();
        return;
    }
    TiXmlHandle docHandle(&doc);
    TiXmlHandle projectHandle = docHandle.FirstChildElement("Project");
    proj = new SoLIMProject();
    proj->workingDir=workingDir;
    proj->projFilename = projectFile.toStdString();
    proj->projName = projectHandle.ToElement()->Attribute("Name");
    if(projectHandle.FirstChildElement("StudyArea").ToElement()->GetText())
        proj->studyArea = projectHandle.FirstChildElement("StudyArea").ToElement()->GetText();
    else
        proj->studyArea = "";

    initModel();
    // read gis data
    TiXmlHandle gisDataHandle = projectHandle.FirstChildElement("GISData");
    for(TiXmlElement* layer = gisDataHandle.FirstChildElement("Layer").ToElement();
        layer; layer = layer->NextSiblingElement("Layer")){
        QFileInfo fileinfo(layer->GetText());
        if(fileinfo.exists()){
            proj->layernames.push_back(layer->Attribute("Name"));
            proj->layertypes.push_back(layer->Attribute("Type"));
            proj->filenames.push_back(layer->GetText());
        } else {
            proj->layernames.push_back(layer->Attribute("Name"));
            proj->layertypes.push_back(layer->Attribute("Type"));
            proj->filenames.push_back("");
            QMessageBox warn;
            string layername = layer->Attribute("Name");
            warn.setText(("Layer \""+layername+"\" does not exist.").c_str());
            warn.setStandardButtons(QMessageBox::Ok);
            warn.exec();
        }
    }
    onGetGisData();
    // add prototypes
    string tmpBaseName;
    TiXmlHandle prototypesHandle = projectHandle.FirstChildElement("Prototypes");

    for(TiXmlElement* prototypeBase = prototypesHandle.FirstChildElement("PrototypeBase").ToElement();
        prototypeBase; prototypeBase = prototypeBase->NextSiblingElement()){
        proj->prototypeBaseNames.push_back(prototypeBase->Attribute("Basename"));
        proj->prototypeBaseTypes.push_back(prototypeBase->Attribute("Source"));
        readPrototype(prototypeBase);
    }
    onGetPrototype();
    // set results
    TiXmlHandle resultHandle = projectHandle.FirstChildElement("Results");
    for(TiXmlElement* result = resultHandle.FirstChildElement("ResultFile").ToElement();
        result; result = result->NextSiblingElement("ResultFile")){
        proj->results.push_back(result->GetText());
    }
    onInferResults();
    projectSaved = true;

}

void MainWindow::on_actionDefine_Study_Area_triggered(){
    if(proj==nullptr) return;
    SimpleDialog editStudyArea(SimpleDialog::EDITSTUDYAREA,proj,this);
    editStudyArea.exec();
    workingDir=proj->workingDir;
    proj->studyArea = editStudyArea.lineEdit2.toStdString();
    QStandardItem *studyarea = new QStandardItem(("Study area: "+proj->studyArea).c_str());
    studyarea->setFlags(studyarea->flags() ^ Qt::ItemIsEditable);
    model->item(0,0)->setChild(0,0, studyarea);
    projectSaved = false;
}
void MainWindow::on_action_infer_triggered(){
    if(proj){
        mapInference *infer= new mapInference(proj,this);
        infer->show();
        connect(infer,SIGNAL(finished(int)),this,SLOT(onInferResults()));
    }
}


void MainWindow::on_actionView_Data_triggered(){
    string filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open Raster File"),
                                                   workingDir,
                                                   tr("Raster file(*.tif *.3dr *.img *.sdat *.bil *.bin *.tiff)")).toStdString();
    if(!filename.empty()){
        workingDir=QFileInfo(filename.c_str()).absoluteDir().absolutePath();
        bool opened = drawLayer(filename);
        if(!opened){
            QMessageBox warn;
            warn.setText("File open failed! Please make sure the filename and path to file does not contain non-English characters.");
            warn.exec();
        }
    }
}

//==================================== Project tree slots ==========================================
void MainWindow::onSelectionChanged(const QItemSelection& current,const QItemSelection& previous){
    QModelIndex index = current.indexes().at(0);
    if(myGraphicsView->editFreehandRule == true||myGraphicsView->editEnumRule == true){
        if(myGraphicsView->membership->getKnotNum()>2) {
            QMessageBox warn;
            warn.setText("Do you want to save edit?");
            warn.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
            int ret = warn.exec();
            if(ret == QMessageBox::Ok) saveEditRuleChange();
        }
    }
    myGraphicsView->editFreehandRule = false;
    myGraphicsView->editEnumRule = false;
    if(index.isValid()&&index.parent().data().toString().compare("Results")==0){
        string filename = index.data().toString().toStdString();
        drawLayer(filename);
    }
    else if(index.isValid()&&index.parent().data().toString().compare("Covariates")==0){
        string layername=index.data().toString().toStdString();
        string filename="";
        for(size_t k = 0;k < proj->layernames.size();k++){
            if(layername==proj->layernames[k].c_str()){
                filename=proj->filenames[k];
                break;
            }
        }
        drawLayer(filename);
    }
    else if(index.isValid()&&index.data().toString().compare("Membership Function")==0){
        string prototype = index.parent().parent().parent().data().toString().toStdString();
        int prefixLength = 11;
        int idPrefixLength = 14;
        int basePrefixLength = 16;
        currentBaseName = index.parent().parent().parent().parent().data().toString().toStdString().substr(basePrefixLength);
        currentProtoName = prototype.substr(idPrefixLength);
        currentLayerName = index.parent().data().toString().toStdString().substr(prefixLength);
        drawMembershipFunction();
    }
}

void MainWindow::onCustomContextMenu(const QPoint & point){
    QModelIndex index = projectView->indexAt(point);
    if(index.isValid()&&index.data().toString().compare("Prototypes")==0){
        prototypesMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    else if(index.isValid()&&index.data().toString().compare("Covariates")==0){
        if(index.parent().data().toString().compare(proj->projName.c_str())==0)
            gisDataMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    else if (index.isValid()&&index.parent().data().toString().compare("Covariates")==0){
        if(index.parent().parent().data().toString().startsWith("Prototype ID:")==false){
            currentLayerName=index.data().toString().toStdString();
            gisLayerMenu->exec(projectView->viewport()->mapToGlobal(point));
        }
    }
    else if(index.isValid()&&index.parent().data().toString().compare("Prototypes")==0){
        currentBaseName=index.data().toString().mid(16).toStdString();
        size_t i = 0;
        while(i < proj->prototypeBaseNames.size()){
            if(proj->prototypeBaseNames[i]==currentBaseName)
                break;
            i++;
        }
        if(proj->prototypeBaseTypes[i]=="EXPERT"){
            addRules->setVisible(true);
            addProtoExpert->setVisible(true);
        }
        else{
            addRules->setVisible(false);
            addProtoExpert->setVisible(false);
        }
        prototypeBaseMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    else if(index.isValid()&&index.data().toString().startsWith("Prototype ID:")){
        currentProtoName = index.data().toString().mid(14).toStdString();
        currentBaseName=index.parent().data().toString().mid(16).toStdString();
        prototypeMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
    else if(index.isValid()&&index.data().toString().startsWith("Membership")){
        int prefixLength = 11;
        int idPrefixLength = 14;
        int basePrefixLength = 16;
        currentBaseName = index.parent().parent().parent().parent().data().toString().toStdString().substr(basePrefixLength);
        currentProtoName = index.parent().parent().parent().data().toString().toStdString().substr(idPrefixLength);
        currentLayerName = index.parent().data().toString().toStdString().substr(prefixLength);
        if(myGraphicsView->editFreehandRule == true || myGraphicsView->editEnumRule == true){
            editRule->setEnabled(false);
            saveRule->setEnabled(true);
            resetRule->setEnabled(true);
        } else {
            editRule->setEnabled(true);
            saveRule->setEnabled(false);
            resetRule->setEnabled(false);
        }
        membershipMenu->exec(projectView->viewport()->mapToGlobal(point));
    }
}

void MainWindow::onAddGisData(){
    SimpleDialog addGisData(SimpleDialog::ADDGISDATA,proj, this);
    addGisData.exec();
    workingDir=proj->workingDir;
    if(addGisData.filename.isEmpty()){
        return;
    }
    for(size_t i = 0;i<proj->filenames.size();i++){
        if(proj->filenames[i]==addGisData.filename.toStdString()){
            QMessageBox warning;
            warning.setText("This file already exists in covariates.");
            warning.exec();
            return;
        }
        if(proj->layernames[i]==addGisData.covariate.toStdString()){
            QMessageBox warning;
            warning.setText("This covariate name already exists. Please rename the covariate.");
            warning.exec();
            return;
        }
    }
    proj->filenames.push_back(addGisData.filename.toStdString());
    proj->layernames.push_back(addGisData.covariate.toStdString());
    proj->layertypes.push_back(addGisData.datatype);
    drawLayer(addGisData.filename.toStdString());
    onGetGisData();
}

void MainWindow::on_actionAdd_prototypes_from_samples_triggered(){
    AddPrototypeBase*getPrototype = new AddPrototypeBase(AddPrototypeBase::SAMPLE,proj,this);
    getPrototype->exec();
    // show prototypes
    //connect(getPrototype,SIGNAL(finished(int)),this,SLOT(onGetPrototype()));
    onGetPrototype();
    if(getPrototype->addedLayer>0) onGetGisData();
    workingDir=proj->workingDir;
    projectSaved = false;
}

void MainWindow::on_actionAdd_prototypes_from_expert_triggered(){
    SimpleDialog *addPrototypeBase = new SimpleDialog(SimpleDialog::ADDPROTOTYPEBASE,proj,this);
    addPrototypeBase->exec();
    workingDir=proj->workingDir;
    if(!addPrototypeBase->lineEdit2.isEmpty()){
        QString basename=addPrototypeBase->lineEdit2;
        for(size_t i = 0;i<proj->prototypeBaseNames.size();i++){
            if(proj->prototypeBaseNames[i]==basename.toStdString()){
                QMessageBox warn;
                warn.setText("Prototype base "+basename+" already exists.");
                warn.exec();
                return;
            }
        }
        proj->prototypeBaseNames.push_back(basename.toStdString());
        proj->prototypeBaseTypes.push_back("EXPERT");
        QStandardItem* protobase = new QStandardItem("Prototype Base: "+basename);
        protobase->setFlags(protobase->flags() ^ Qt::ItemIsEditable);
        prototypeChild->setChild(prototypeChild->rowCount(),0,protobase);
        projectView->expand(prototypeChild->index());
        if(addPrototypeBase->nextFlag){
            currentBaseName=basename.toStdString();
            onCreatePrototypeFromExpert();
        }
    }
}

void MainWindow::onCreatePrototypeFromExpert(){
    for (size_t i = 0;i<proj->prototypeBaseNames.size();i++){
        if(proj->prototypeBaseNames[i]==currentBaseName){
            if(proj->prototypeBaseTypes[i]!="EXPERT"){
                QMessageBox warn;
                warn.setText("Prototype from expert cannot be added into prototype bases created from samples or data mining.");
                warn.exec();
                return;
            }
        }
    }
    if(addRule){ addRule->close(); delete addRule;}
    addRule = new AddPrototype_Expert(proj,-1,currentBaseName,this);
    addRule->show();
    connect(addRule,SIGNAL(addlayer()),this,SLOT(onGetGisData()));
    connect(addRule,SIGNAL(updatePrototype()),this,SLOT(onGetPrototype()));
}

void MainWindow::onAddRules(){
    for (size_t i = 0;i<proj->prototypes.size();i++){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName
                &&proj->prototypes[i].prototypeID==currentProtoName){
            if(addRule){ addRule->close(); delete addRule;}
            addRule = new AddPrototype_Expert(proj,i,"",this);
            addRule->show();
            connect(addRule,SIGNAL(addlayer()),this,SLOT(onGetGisData()));
            connect(addRule,SIGNAL(updatePrototype()),this,SLOT(onGetPrototype()));
        }
    }
}

void MainWindow::onDeletePrototypeBase(){
    vector<Prototype>::iterator it = proj->prototypes.begin();
    while (it!=proj->prototypes.end()){
        if((*it).prototypeBaseName==currentBaseName){
            it=proj->prototypes.erase(it);
        } else ++it;
    }
    vector<string>::iterator it_base = proj->prototypeBaseNames.begin();
    vector<string>::iterator it_type = proj->prototypeBaseTypes.begin();
    while(it_base!=proj->prototypeBaseNames.end()){
        if((*it_base)==currentBaseName){
            it_base=proj->prototypeBaseNames.erase(it_base);
            it_type=proj->prototypeBaseTypes.erase(it_type);
        } else {
            ++it_base;
            ++it_type;
        }
    }
    onGetPrototype();
}
void MainWindow::onDeletePrototype(){
    vector<Prototype>::iterator it = proj->prototypes.begin();
    while (it!=proj->prototypes.end()){
        if((*it).prototypeBaseName==currentBaseName&&(*it).prototypeID==currentProtoName){
            it=proj->prototypes.erase(it);
        } else ++it;
    }
    onGetPrototype();
}

void MainWindow::onUpdatePrototypeFromExpert(const Prototype*prop){
    for(int i =0; i<prototypeChild->rowCount();i++){
        if(prototypeChild->child(i,0)->text().endsWith(currentBaseName.c_str())){
            QStandardItem *editExpertBase=prototypeChild->child(i,0);
            for(int j =0;j<editExpertBase->rowCount();j++){
                if(editExpertBase->child(j,0)->text().mid(14).toStdString()==prop->prototypeID){
                    QStandardItem*prototype=editExpertBase->child(j,0);
                    prototype->setFlags(prototype->flags() ^ Qt::ItemIsEditable);
                    prototype->setRowCount(0);
                    prototype->setColumnCount(1);
                    QStandardItem* source = new QStandardItem("Source: EXPERT");
                    source->setFlags(source->flags() ^ Qt::ItemIsEditable);
                    prototype->setChild(0,0,source);
                    if(prop->properties.size()>0){
                        QStandardItem*properties=new QStandardItem("Properties");
                        properties->setFlags(properties->flags() ^ Qt::ItemIsEditable);
                        prototype->setChild(prototype->rowCount(),0,properties);
                        for(size_t k = 0;k<prop->properties.size();k++) {
                            string property = prop->properties[k].propertyName;
                            if(prop->properties[k].soilPropertyType==solim::CATEGORICAL){
                                property += " (category): " + to_string(int(prop->properties[k].propertyValue));
                            } else
                                property += " (property): " + to_string(prop->properties[k].propertyValue);
                            QStandardItem*property_item=new QStandardItem(property.c_str());
                            property_item->setFlags(property_item->flags() ^ Qt::ItemIsEditable);
                            properties->setChild(properties->rowCount(),0,property_item);
                        }
                    }
                    if(prop->envConditionSize>0){
                        QStandardItem* covariates = new QStandardItem("Covariates");
                        covariates->setFlags(covariates->flags() ^ Qt::ItemIsEditable);
                        prototype->setChild(prototype->rowCount(),0,covariates);
                        covariates->setColumnCount(1);
                        for(int j = 0; j<prop->envConditionSize;j++){
                            string cov = "Covariate: " + prop->envConditions[j].covariateName;
                            covariates->setRowCount(covariates->rowCount()+1);
                            QStandardItem *covItem = new QStandardItem(cov.c_str());
                            covItem->setFlags(covItem->flags() ^ Qt::ItemIsEditable);
                            covariates->setChild(covariates->rowCount()-1,0,covItem);
                            covItem->setColumnCount(1);
                            double typicalValue = prop->envConditions[j].typicalValue;
                            if(fabs(typicalValue-NODATA)>VERY_SMALL){
                                string typicalV = "Typical value: "+to_string(typicalValue);
                                QStandardItem *typicalV_item = new QStandardItem(typicalV.c_str());
                                typicalV_item->setFlags(typicalV_item->flags() ^ Qt::ItemIsEditable);
                                covItem->setChild(covItem->rowCount(),0,typicalV_item);
                            }
                            // set membership function
                            QStandardItem *membershipf = new QStandardItem("Membership Function");
                            membershipf->setFlags(membershipf->flags() ^ Qt::ItemIsEditable);
                            covItem->setChild(covItem->rowCount(),0,membershipf);
                        }
                    }
                }
            }
        }
    }
}
void MainWindow::on_actionAdd_prototypes_from_Data_Mining_triggered(){
    AddPrototypeBase*getPrototype = new AddPrototypeBase(AddPrototypeBase::MAP,proj,this);
    getPrototype->exec();
    workingDir=proj->workingDir;
    projectSaved = false;
    onGetPrototype();
}

void MainWindow::onImportPrototypeBase(){
    QString basefilename=QFileDialog::getOpenFileName(this,tr("Open prototype base file"),workingDir,tr("(*.csv *.xml)"));
    if(basefilename.isEmpty())  return;
    workingDir=QFileInfo(basefilename).absoluteDir().absolutePath();
    if(basefilename.endsWith(".csv",Qt::CaseInsensitive)){
        QFile basefile(basefilename);
        if(!basefile.open(QFile::ReadOnly)){
            warn("File open failed!");
            return;
        }
        QTextStream *out = new QTextStream(&basefile);
        QStringList lines = out->readAll().split("\n");
        if(lines.size()<1)  return;
        QStringList basenames = lines[0].split(",");
        if(basenames[0]!="basename"){ warn("Wrong file format"); return; }
        string basename = basenames[1].toStdString();
        if(baseExistsWarning(basename)) return;
        proj->prototypeBaseNames.push_back(basename);
        if(basenames[2]!="source"){ warn("Wrong file format"); return; }
        string source = basenames[3].toStdString();
        proj->prototypeBaseTypes.push_back(source);
        solim::PrototypeSource protoSource = solim::getSourceFromString(source);

        for(int i =1;i<lines.size();i++){
            if(lines[i].isEmpty()) break;
            QStringList properties = lines[i].split(",");
            if(properties[0]!="prototype"){ warn("Wrong file format"); return; }
            Prototype proto;
            proto.prototypeBaseName=basename;
            proto.prototypeID = properties[1].toStdString();
            proto.source = protoSource;
            if(properties[2]!="properties_num"){ warn("Wrong file format"); return; }
            int properties_num = properties[3].toInt();
            int loc = 4;
            for(int k=0;k<properties_num;k++){
                proto.addProperties(properties[loc].toStdString(),properties[loc+1].toDouble());
                loc += 2;
            }
            if(properties[loc]!="covariates_num"){ warn("Wrong file format"); return; }
            int cov_num = properties[loc+1].toInt();
            loc+=2;
            for(int k = 0;k<cov_num;k++){
                if(properties[loc]!="covariate_name"){ warn("Wrong file format"); return; }
                string covname = properties[loc+1].toStdString();
                if(properties[loc+2]!="datatype"){ warn("Wrong file format"); return; }
                solim::DataTypeEnum datatype = solim::getDatatypeFromString(properties[loc+3].toStdString());
                if(properties[loc+6]!="range"){ warn("Wrong file format"); return; }
                int range = properties[loc+7].toInt();
                if(properties[loc+8]!="Node_num"){ warn("Wrong file format"); return; }
                int node_num = properties[loc+9].toDouble();
                if(properties[loc+10]!="Membership_function"){ warn("Wrong file format"); return; }
                loc+=11;
                QStringList coords;
                for(int n = 0;n<node_num;n++){
                    coords.push_back(properties[loc]);
                    ++loc;
                }
                string coords_str = coords.join(",").toStdString();
                proto.addConditions(solim::Curve(covname, datatype, node_num, coords_str, range));
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
        proj->prototypeBaseNames.push_back(prototypeBaseHandle.ToElement()->Attribute("Source"));
        readPrototype(prototypeBaseHandle.ToElement());
        onGetPrototype();
        projectSaved=false;
    }
}

void MainWindow::onRenamePrototypeBase(){
    proj->currentBaseName = currentBaseName;
    SimpleDialog *changeBaseName = new SimpleDialog(SimpleDialog::CHANGEBASENAME,proj,this);
    changeBaseName->exec();
    string newname = changeBaseName->lineEdit2.toStdString();
    if(!newname.empty()){
        for(size_t i =0; i<proj->prototypeBaseNames.size();i++){
            if(currentBaseName==proj->prototypeBaseNames[i]){
                proj->prototypeBaseNames[i]=newname;
                for(int i =0; i<prototypeChild->rowCount();i++){
                    if(prototypeChild->child(i,0)->text().endsWith(currentBaseName.c_str())){
                        QStandardItem *editExpertBase=prototypeChild->child(i,0);
                        editExpertBase->setText(("Prototype Base: "+newname).c_str());
                    }
                }
                return;
            }
        }
    }
}

void MainWindow::onChangeCovName(){
    if(proj->prototypes.size()<1) return;
    size_t i = 0;
    while(i < proj->prototypes.size()){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName)
            break;
        i++;
    }
    class changeCovName changeName(&(proj->prototypes[i]),this);
    changeName.exec();
    if(changeName.isChanged){
        for(size_t i =0;i<proj->prototypes.size();i++){
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

void MainWindow::onDeleteGisLayer() {
    for(size_t i = 0;i<proj->layernames.size();i++){
        if(currentLayerName==proj->layernames[i]){
            if(graphicFilename==proj->filenames[i])
                myGraphicsView->getScene()->clear();
            proj->layernames.erase(proj->layernames.begin()+i);
            proj->filenames.erase(proj->filenames.begin()+i);
            proj->layertypes.erase(proj->layertypes.begin()+i);
            projectSaved = false;
            break;
        }
    }
    onGetGisData();
}

void MainWindow::onModifyCovFile() {
    for(size_t i = 0;i<proj->layernames.size();i++){
        if(currentLayerName==proj->layernames[i]){
            SimpleDialog modifyGisData(SimpleDialog::MODIFYGISDATA,proj, this);
            modifyGisData.exec();
            workingDir=proj->workingDir;
            if(modifyGisData.filename.isEmpty()){
                return;
            } else {
               proj->filenames[i]=modifyGisData.filename.toStdString();
               projectSaved = false;
            }
            break;
        }
    }
    onGetGisData();
}

void MainWindow::onModifyCovName(){
    for(size_t i = 0;i<proj->layernames.size();i++){
        if(currentLayerName==proj->layernames[i]){
            SimpleDialog modifyGisName(SimpleDialog::MODIFYLAYERNAME,proj, this);
            modifyGisName.exec();
            workingDir=proj->workingDir;
            if(modifyGisName.covariate.isEmpty()){
                return;
            } else {
               proj->layernames[i]=modifyGisName.covariate.toStdString();
               projectSaved = false;
            }
            break;
        }
    }
    onGetGisData();
}

void MainWindow::onSavePrototypeBase(){
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save prototype base as"),
                                                    workingDir,
                                                    tr("(*.xml)"));
    workingDir=QFileInfo(filename).absoluteDir().absolutePath();
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc->LinkEndChild(pDeclaration);
    TiXmlElement *prototypes_node = new TiXmlElement("PrototypeBase");
    prototypes_node->SetAttribute("Basename",currentBaseName.c_str());
    for(size_t i = 0; i< proj->prototypeBaseNames.size();i++){
        if(proj->prototypeBaseNames[i]==currentBaseName){
            prototypes_node->SetAttribute("Source", proj->prototypeBaseTypes[i].c_str());
        }
    }
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
                                                    workingDir,
                                                    tr("(*.csv)"));
    workingDir=QFileInfo(filename).absoluteDir().absolutePath();
    if(filename.isEmpty()){
        warn("Please input file name!");
        return;
    }
    QFile basefile(filename);
    if(!basefile.open(QFile::WriteOnly)){
        warn("File open failed!");
        return;
    }
    if(currentBaseName.find(',')!=std::string::npos){
        warn("Please do not use ',' in prototype base name");
        return;
    }
    QTextStream stream(&basefile);
    stream<<"basename,"<<currentBaseName.c_str()<<",source,"<<solim::PrototypeSource_str[proj->prototypes[0].source]<<"\n";
    for(size_t i = 0;i<proj->prototypes.size();i++){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName){
            solim::Prototype *proto = &(proj->prototypes[i]);
            stream<<"prototype,"<<proto->prototypeID.c_str()<<",properties_num,"<<QString::number(proto->properties.size())<<",";
            for(size_t j=0;j<proto->properties.size();j++){
                if(proto->properties[j].propertyName.find(',')!=std::string::npos){
                    warn("Please do not use ',' in prototype property name");
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
    gisDataChild->setRowCount(proj->layernames.size());
    QStandardItem *item;
    for(size_t i = 0;i<proj->filenames.size();i++){
        item = new QStandardItem(proj->layernames[i].c_str());
        item->setFlags(item->flags()^Qt::ItemIsEditable);
        gisDataChild->setChild(i,0,item);
        gisDataChild->setRowCount(proj->filenames.size());
        if(proj->filenames[i]!=""){
            item = new QStandardItem(("Filename: "+proj->filenames[i]).c_str());
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            gisDataChild->child(i)->setChild(0,0,item);
            item = new QStandardItem(("Type: "+proj->layertypes[i]).c_str());
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            gisDataChild->child(i)->setChild(1,0,item);
        } else {
            item = new QStandardItem(("Type: "+proj->layertypes[i]).c_str());
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            gisDataChild->child(i)->setChild(1,0,item);
        }
    }
}

void MainWindow::onGetPrototype(){
    prototypeChild->setColumnCount(1);
    prototypeChild->setRowCount(proj->prototypeBaseNames.size());
    for(size_t i =0;i<proj->prototypeBaseNames.size();i++){
        string prototypebase = proj->prototypeBaseNames[i];
        QStandardItem *prototypeBase = new QStandardItem(("Prototype Base: "+prototypebase).c_str());
        prototypeBase->setFlags(prototypeBase->flags()^Qt::ItemIsEditable);
        prototypeChild->setChild(i,0,prototypeBase);
        for(vector<Prototype>::iterator it = proj->prototypes.begin(); it!=proj->prototypes.end();it++){
            if((*it).prototypeBaseName==prototypebase){
                QStandardItem* prototype = new QStandardItem(("Prototype ID: "+(*it).prototypeID).c_str());
                prototype->setFlags(prototype->flags()^Qt::ItemIsEditable);
                prototypeBase->setChild(prototypeBase->rowCount(),0,prototype);
                prototype->setColumnCount(1);
                QStandardItem *source = new QStandardItem(("Source: "+string(solim::PrototypeSource_str[(*it).source])).c_str());
                source->setFlags(source->flags() ^ Qt::ItemIsEditable);
                prototype->setChild(0,0,source);
                QStandardItem* properties = new QStandardItem("Properties");
                properties->setFlags(properties->flags()^Qt::ItemIsEditable);
                prototype->setChild(1,0,properties);
                properties->setColumnCount(1);
                for(size_t i = 0;i<(*it).properties.size();i++) {
                    string property = (*it).properties[i].propertyName;
                    double value = (*it).properties[i].propertyValue;
                    if((*it).properties[i].soilPropertyType==solim::CATEGORICAL){
                        property += " (category): " + to_string(int(value));
                    } else {
                        if(fabs(value-int(value)) < VERY_SMALL)
                            property += " (continuous): " + to_string(int(value));
                        else
                            property += " (continuous): " + to_string(value);
                    }
                    properties->setRowCount(properties->rowCount()+1);
                    QStandardItem *property_item = new QStandardItem(property.c_str());
                    property_item->setFlags(property_item->flags()^Qt::ItemIsEditable);
                    properties->setChild(properties->rowCount()-1,0,property_item);
                }
                QStandardItem* covariates = new QStandardItem("Covariates");
                covariates->setFlags(covariates->flags()^Qt::ItemIsEditable);
                prototype->setChild(2,0,covariates);
                covariates->setColumnCount(1);
                for(int j = 0; j<(*it).envConditionSize;j++){
                    string cov = "Covariate: " + (*it).envConditions[j].covariateName;
                    covariates->setRowCount(covariates->rowCount()+1);
                    QStandardItem *covItem = new QStandardItem(cov.c_str());
                    covItem->setFlags(covItem->flags()^Qt::ItemIsEditable);
                    covariates->setChild(covariates->rowCount()-1,0,covItem);
                    covItem->setColumnCount(1);
                    double typicalValue = (*it).envConditions[j].typicalValue;
                    if(fabs(typicalValue-NODATA)>VERY_SMALL){
                        string typicalV = "Typical value: "+to_string(typicalValue);
                        QStandardItem * typ_v = new QStandardItem(typicalV.c_str());
                        typ_v->setFlags(typ_v->flags()^Qt::ItemIsEditable);
                        covItem->setChild(covItem->rowCount(),0,typ_v);
                    }
                    // set membership function
                    QStandardItem * mem_f = new QStandardItem("Membership Function");
                    mem_f->setFlags(mem_f->flags()^Qt::ItemIsEditable);
                    covItem->setChild(covItem->rowCount(),0,mem_f);
                }
            }
        }
    }
    ui->action_infer->setEnabled(true);
}

void MainWindow::onInferResults(){
    if(!proj)
        return;
    if(resultChild->rowCount()==proj->results.size()){
        return;
    }
    resultChild->setColumnCount(1);
    resultChild->setRowCount(0);
    for(size_t i = 0; i<proj->results.size();i++){
        resultChild->setRowCount(resultChild->rowCount()+1);
        QStandardItem * res = new QStandardItem(proj->results[i].c_str());
        res->setFlags(res->flags()^Qt::ItemIsEditable);
        resultChild->setChild(resultChild->rowCount()-1,0,res);
    }
    projectView->expand(resultChild->index());
    projectSaved = false;
    if(proj->currentResultName != "") {
        drawLayer(proj->currentResultName);
        proj->currentResultName = "";
    }
}

//=========================== Main Graphics View function===============================
bool MainWindow::drawLayer(string filename){
    graphicFilename=filename;
    if(filename.empty()) return false;
    QFileInfo fileinfo(filename.c_str());
    if(!fileinfo.exists()) return false;
    imgFilename = filename;
    string imagename = filename+".png";
    QTextCodec *code = QTextCodec::codecForName("UTF-8");
    QString imagename_q = QString::fromStdString(code->fromUnicode(QString(imagename.c_str())).data());
    img = new QImage(imagename_q);
    lyr = new BaseIO(filename);
    if(!lyr->isOpened()) return false;
    imgMax = lyr->getDataMax();
    imgMin = lyr->getDataMin();
    if(img->isNull()){
        ui->statusBar->showMessage("Loading Data...");
        delete img;
        createImgThread.start();

    } else {
        myGraphicsView->showImage = true;
        myGraphicsView->showMembership=false;
        zoomToolBar->setVisible(true);
        resetRangeToolBar->setVisible(false);
        myGraphicsView->getScene()->clear();
        ui->statusBar->showMessage("coordinate: ("+QString::number(lyr->getXMin())+", "
                                   +QString::number(lyr->getYMin())+") (TopLeft)");

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
    return true;
}

void MainWindow::createImg(){
    string imagename = lyr->getFilename()+".png";
    QTextCodec *code = QTextCodec::codecForName("UTF-8");
    QString imagename_q = QString::fromStdString(code->fromUnicode(QString(imagename.c_str())).data());
    int stretch=1;
    int xsize=lyr->getXSize();
    int ysize=lyr->getYSize();
    if(xsize*ysize>1000000){
        stretch=sqrt(xsize*ysize/1000000);
        xsize=xsize/stretch;
        if(lyr->getXSize()%stretch>0) xsize=xsize+1;
        ysize=ysize/stretch;
        if(lyr->getYSize()%stretch>0) ysize=ysize+1;
    }
    float* pafScanline = new float[xsize*ysize];
    unsigned char*imgData = new unsigned char[xsize*ysize];
    if(stretch==1)
        lyr->read(0,0,lyr->getYSize(),lyr->getXSize(),pafScanline);
    else {
        lyr->blockInit();
        float* tmp = new float[lyr->getBlockX()*lyr->getBlockY()];
        int k = 0;
        for(int i = 0;i<lyr->getBlockSize();i++){
            int mode=(stretch-i*lyr->getBlockY()%stretch)%stretch;
            int blockX = lyr->getBlockX();
            int blockY = lyr->getBlockY();
            if(i==lyr->getBlockSize()-1&i>0){
                blockY=lyr->getYSize()-i*lyr->getBlockY();
            }
            lyr->read(0,i*lyr->getBlockY(),blockY,blockX,tmp);
            for(int n=mode;n<blockY;n+=stretch){
                for(int m=0;m<blockX;m+=stretch){
                    pafScanline[k]=tmp[n*blockX+m];
                    k++;
                }
            }
        }
    }
    float tmp = pafScanline[int((ysize/2)*xsize+xsize/2)];
    float range = 254.0/(imgMax-imgMin);
    for(int i = 0; i<xsize*ysize;i++){
        float value = pafScanline[i];
        if(fabs(value-NODATA)<VERY_SMALL||value<NODATA){
            imgData[i]=255;
        }else{
            imgData[i] = (value-imgMin)*range;
        }
    }
    img = new QImage(imgData, xsize,ysize,xsize, QImage::Format_Grayscale8);
    img->save(imagename_q);
    img->load(imagename_q);
    if(pafScanline)
        delete []pafScanline;
    if(imgData)
        delete []imgData;
    delete lyr;
    createImgThread.exit(0);
}

void MainWindow::finishedCreateImg(){
    myGraphicsView->showImage = true;
    myGraphicsView->showMembership=false;
    zoomToolBar->setVisible(true);
    resetRangeToolBar->setVisible(false);
    myGraphicsView->getScene()->clear();
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
    ui->statusBar->clearMessage();
}

void MainWindow::onResetRange(){
    SimpleDialog resetRangeDialog(SimpleDialog::RESETRANGE,proj,this);\
    resetRangeDialog.exec();
    bool max_ok, min_ok;
    float max = resetRangeDialog.lineEdit2.toFloat(&max_ok);
    float min = resetRangeDialog.lineEdit1.toFloat(&min_ok);
    if(max_ok && min_ok) drawMembershipFunction(max,min);
}

void MainWindow::drawMembershipFunction(float max, float min, solim::Curve *c){
    bool predefined_range = false;
    if((max-NODATA)>VERY_SMALL && (min-NODATA)>VERY_SMALL)
        predefined_range=true;
    graphicFilename="";
    zoomToolBar->setVisible(false);
    resetRangeToolBar->setVisible(true);
    myGraphicsView->showImage = false;
    myGraphicsView->dataDetailsView->setModel(new QStandardItemModel(myGraphicsView->dataDetailsView));
    myGraphicsView->getScene()->clear();
    int protoPos = -1;
    int covPos = -1;
    for(size_t i = 0;i<proj->prototypes.size();i++){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName
                && proj->prototypes[i].prototypeID==currentProtoName){
            protoPos = i;
            for(int j = 0; j<proj->prototypes[i].envConditionSize;j++){
                if(proj->prototypes[i].envConditions[j].covariateName==currentLayerName){
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
    if(c==nullptr){
        myGraphicsView->showMembership=true;
        myGraphicsView->membership->changeCurve(&(proj->prototypes[protoPos].envConditions[covPos]));
    } else {
        myGraphicsView->showMembership=true;
        myGraphicsView->membership->changeCurve(c);
    }    
    myGraphicsView->getScene()->setSceneRect(0,0,myGraphicsView->width()*0.9,myGraphicsView->height()*0.9);
    QPen curvePen(Qt::black);
    QPen axisPen(Qt::gray);
    axisPen.setWidth(2);
    curvePen.setWidth(1);
    string coords = myGraphicsView->membership->getCoords();
    vector<string> xycoord;
    solim::ParseStr(coords, ',', xycoord);
    int iKnotNum = xycoord.size();
    double xmin,xmax;
    vector<string> startCoord;
    vector<string> endCoord;
    solim::ParseStr(xycoord[0], ' ', startCoord);
    solim::ParseStr(xycoord[iKnotNum-1], ' ', endCoord);
    xmin = atof(startCoord[0].c_str());
    xmax = atof(endCoord[0].c_str());
    int sceneWidth = myGraphicsView->getScene()->width();
    int sceneHeight = myGraphicsView->getScene()->height();
    double scale = fabs(xmax)>fabs(xmin)?fabs(xmax):fabs(xmin);
    if(scale<10) scale = (int(scale)+1)*2;
    else scale = (int(scale/10)+1)*10*2;
    int margin = 0.5*scale;
    // add info
    QGraphicsTextItem *covNameText = myGraphicsView->getScene()->addText(string("Covariate: "+currentLayerName).c_str());
    covNameText->setFont(QFont("Times", 10));
    covNameText->setPos(0.02*sceneWidth, 0.02*sceneHeight);
    string source = solim::PrototypeSource_str[proj->prototypes[protoPos].source];
    QGraphicsTextItem *covSourceText = myGraphicsView->getScene()->addText(string("Source: "+source).c_str());
    covSourceText->setFont(QFont("Times", 10));
    covSourceText->setPos(0.02*sceneWidth, 0.02*sceneHeight+20);
    string typicalValue = to_string(myGraphicsView->membership->typicalValue);
    QGraphicsTextItem *typicalValueText=myGraphicsView->getScene()->addText(string("Typical value: "+typicalValue).c_str());
    typicalValueText->setFont(QFont("Times", 10));
    typicalValueText->setPos(0.02*sceneWidth, 0.02*sceneHeight+40);

    myGraphicsView->getScene()->addLine(0.05*sceneWidth,0.85*sceneHeight,0.85*sceneWidth,0.85*sceneHeight,axisPen);
    myGraphicsView->getScene()->addLine(0.85*sceneWidth,0.85*sceneHeight+1,0.85*sceneWidth-3,0.85*sceneHeight+4,axisPen);
    myGraphicsView->getScene()->addLine(0.85*sceneWidth,0.85*sceneHeight,0.85*sceneWidth-3,0.85*sceneHeight-3,axisPen);
    myGraphicsView->getScene()->addLine(0.80*sceneWidth,0.85*sceneHeight,0.80*sceneWidth,0.85*sceneHeight-3,axisPen);
    myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.85*sceneHeight-3,axisPen);
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
    // drawing coordinates
    if(predefined_range) {
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.1*sceneHeight,axisPen);
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth-3,0.1*sceneHeight+3,axisPen);
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth+3,0.1*sceneHeight+3,axisPen);
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.15*sceneHeight,0.10*sceneWidth+3,0.15*sceneHeight,axisPen);
        yaxis1->setPos(0.10*sceneWidth-15,0.15*sceneHeight-10);
        yaxis0->setPos(0.10*sceneWidth-5,0.85*sceneHeight);
        covNameText->setPos(0.9*sceneWidth-100, 0.02*sceneHeight);
        covSourceText->setPos(0.9*sceneWidth-100, 0.02*sceneHeight+20);
        typicalValueText->setPos(0.9*sceneWidth-100, 0.02*sceneHeight+40);
        yaxisName->setPos(0.10*sceneWidth, 0.1*sceneHeight-20);
        yaxis0->setPos(0.10*sceneWidth-20,0.85*sceneHeight-20);
        QGraphicsTextItem *xaxis0 = myGraphicsView->getScene()->addText(QString::number(min));
        xaxis0->setFont(QFont("Times", 10, QFont::Bold));
        xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
        QGraphicsTextItem *xaxis1 = myGraphicsView->getScene()->addText(QString::number(max));
        xaxis1->setFont(QFont("Times", 10, QFont::Bold));
        xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
        myGraphicsView->curveXMin=min;
        myGraphicsView->curveXMax=max;
        scale = max-min;
    } else if(xmin*xmax<0||xmax<VERY_SMALL){
        // set axis
        myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.85*sceneHeight,0.45*sceneWidth,0.1*sceneHeight,axisPen);
        myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth-3,0.1*sceneHeight+3,axisPen);
        myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth+3,0.1*sceneHeight+3,axisPen);
        // set label
        myGraphicsView->getScene()->addLine(0.45*sceneWidth,0.15*sceneHeight,0.45*sceneWidth+3,0.15*sceneHeight,axisPen);
        QGraphicsTextItem *xaxis1 = myGraphicsView->getScene()->addText(QString::number(margin));
        xaxis1->setFont(QFont("Times", 10, QFont::Bold));
        xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
        QGraphicsTextItem *xaxis0 = myGraphicsView->getScene()->addText(QString::number(-margin));
        xaxis0->setFont(QFont("Times", 10, QFont::Bold));
        xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
        myGraphicsView->curveXMin=-margin;
        myGraphicsView->curveXMax=margin;
        scale = 2*margin;
    } else {
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.1*sceneHeight,axisPen);
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth-3,0.1*sceneHeight+3,axisPen);
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth+3,0.1*sceneHeight+3,axisPen);
        myGraphicsView->getScene()->addLine(0.10*sceneWidth,0.15*sceneHeight,0.10*sceneWidth+3,0.15*sceneHeight,axisPen);
        yaxis1->setPos(0.10*sceneWidth-15,0.15*sceneHeight-10);
        yaxis0->setPos(0.10*sceneWidth-5,0.85*sceneHeight);
        covNameText->setPos(0.9*sceneWidth-100, 0.02*sceneHeight);
        covSourceText->setPos(0.9*sceneWidth-100, 0.02*sceneHeight+20);
        typicalValueText->setPos(0.9*sceneWidth-100, 0.02*sceneHeight+40);
        yaxisName->setPos(0.10*sceneWidth, 0.1*sceneHeight-20);
        if ((xmin>xmax-xmin && xmin>10) || margin-xmax>xmax-xmin) {
            yaxis0->setPos(0.10*sceneWidth-20,0.85*sceneHeight-20);
            int rangemin = int(xmin/10)*10;
            if(margin>5) margin = (int(xmax/10)+1)*10;
            scale=margin-rangemin;
            QGraphicsTextItem *xaxis0 = myGraphicsView->getScene()->addText(QString::number(rangemin));
            xaxis0->setFont(QFont("Times", 10, QFont::Bold));
            xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
            QGraphicsTextItem *xaxis1 = myGraphicsView->getScene()->addText(QString::number(margin));
            xaxis1->setFont(QFont("Times", 10, QFont::Bold));
            xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
            myGraphicsView->curveXMin=rangemin;
            myGraphicsView->curveXMax=margin;
        } else {
            // set label
            QGraphicsTextItem *xaxis1 = myGraphicsView->getScene()->addText(QString::number(margin));
            xaxis1->setFont(QFont("Times", 10, QFont::Bold));
            xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
            scale=margin;
            myGraphicsView->curveXMin=0;
            myGraphicsView->curveXMax=margin;
        }
    }
    myGraphicsView->knotX.clear();
    myGraphicsView->knotY.clear();
    myGraphicsView->knotX.shrink_to_fit();
    myGraphicsView->knotY.shrink_to_fit();
    for(int i = 0; i < myGraphicsView->membership->getKnotNum(); i++){
        myGraphicsView->knotX.push_back(myGraphicsView->membership->vecKnotX[i]);
        myGraphicsView->knotY.push_back(myGraphicsView->membership->vecKnotY[i]);
    }

    double x,y;
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;

    if(myGraphicsView->membership->dataType==solim::CATEGORICAL){
        for(size_t i = 0; i<myGraphicsView->membership->vecKnotX.size();i++){
            double x = myGraphicsView->membership->vecKnotX[i];
            myGraphicsView->getScene()->addLine((x-myGraphicsView->curveXMin)/scale*graphWidth+xStart,yEnd,(x-myGraphicsView->curveXMin)/scale*graphWidth+xStart,yEnd-graphHeight,curvePen);
        }
        return;
    }

    if(myGraphicsView->membership->getKnotNum() < 3)
        return;

    double previousx = myGraphicsView->curveXMin;
    double previousy = myGraphicsView->membership->getOptimality(previousx);
    double range_min = previousx;
    double interval = (myGraphicsView->curveXMax - myGraphicsView->curveXMin)/100.0;

    if(myGraphicsView->membership->dataType==solim::CONTINUOUS){
        for(int i =0;i<100;i++){
            x =previousx+interval;
            y = myGraphicsView->membership->getOptimality(x);
            if(fabs(y+1)<VERY_SMALL ||fabs(previousy+1)<VERY_SMALL)
                continue;
            myGraphicsView->getScene()->addLine((previousx-range_min)/scale*graphWidth+xStart,yEnd-previousy*graphHeight,(x-range_min)/scale*graphWidth+xStart,yEnd-y*graphHeight,curvePen);
            previousx = x;
            previousy = y;
        }
    } else {
        curvePen.setWidth(2);
        for(size_t i = 0;i<myGraphicsView->membership->vecKnotX.size();i++){
            x=myGraphicsView->membership->vecKnotX[i];
            myGraphicsView->getScene()->addLine((x-range_min)/scale*graphWidth+xStart,yEnd,(x+margin)/scale*graphWidth+xStart,yEnd-graphHeight,curvePen);
            QGraphicsTextItem *tag = myGraphicsView->getScene()->addText(QString::number(int(x)));
            tag->setFont(QFont("Times", 8));
            tag->setDefaultTextColor(Qt::blue);
            tag->setPos((int(x)+margin)/scale*graphWidth+xStart-4*tag->toPlainText().size(),yEnd);
        }
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
    // add legend
    QLinearGradient linear(QPoint(5,15),QPoint(5,65));
    linear.setColorAt(0,Qt::white);
    linear.setColorAt(1,Qt::black);
    linear.setSpread(QGradient::PadSpread);
    myGraphicsView->getScene()->addRect(5,15,10,50,QPen(QColor(255,255,255),0),linear);
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
    model->setData(model->index(0,0), proj->projName.c_str(),Qt::DisplayRole);
    QStandardItem *projectname = model->itemFromIndex(model->index(0,0));
    projectname->setFlags(projectname->flags() ^ Qt::ItemIsEditable);
    QStandardItem *studyarea = new QStandardItem(("Study area: "+proj->studyArea).c_str());
    studyarea->setFlags(studyarea->flags() ^ Qt::ItemIsEditable);
    model->item(0,0)->setChild(0,0, studyarea);
    gisDataChild = new QStandardItem("Covariates");
    gisDataChild->setFlags(gisDataChild->flags() ^ Qt::ItemIsEditable);
    gisDataChild->setIcon(QIcon("./imgs/gisdata.svg"));
    model->item(0,0)->setChild(model->item(0,0)->rowCount(),0,gisDataChild);
    prototypeChild = new QStandardItem("Prototypes");
    prototypeChild->setFlags(prototypeChild->flags() ^ Qt::ItemIsEditable);
    prototypeChild->setIcon(QIcon("./imgs/prototype.svg"));
    model->item(0,0)->setChild(model->item(0,0)->rowCount(),0,prototypeChild);
    resultChild = new QStandardItem("Results");
    resultChild->setFlags(resultChild->flags() ^ Qt::ItemIsEditable);
    resultChild->setIcon(QIcon("./imgs/result.svg"));
    model->item(0,0)->setChild(model->item(0,0)->rowCount(),0,resultChild);
    QStandardItem *projects = new QStandardItem("Projects");
    projects->setFlags(projects->flags() ^ Qt::ItemIsEditable);
    model->setHorizontalHeaderItem( 0, projects );

    projectView->setModel(model);
    projectView->expand(model->item(0,0)->index());

    projectSaved = false;
    connect(projectView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&,const QItemSelection&)));
    ui->actionAdd_prototypes_from_samples->setEnabled(true);
    ui->actionAdd_prototypes_from_Data_Mining->setEnabled(true);
    ui->actionAdd_prototypes_from_expert->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSave_as->setEnabled(true);
    ui->actionClose_Project->setEnabled(true);
    ui->actionDefine_Study_Area->setEnabled(true);
}

void MainWindow::initialProjectView(){
    if(projectViewInitialized){
        return;
    }
    projectDock = new QDockWidget(tr("Project"), this);
    projectView = new QTreeView(projectDock);
    projectDock->setFeatures(projectDock->features() & ~QDockWidget::DockWidgetClosable);
    projectDock->setWidget(projectView);
    projectView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(projectView,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(onCustomContextMenu(const QPoint &)));
    connect(projectView, SIGNAL(expanded(QModelIndex)), this, SLOT(onExpanded(QModelIndex)));
    connect(projectView, SIGNAL(collapsed(QModelIndex)), this, SLOT(onExpanded(QModelIndex)));
    // prototype menu
    prototypesMenu = new QMenu(projectView);
    QAction *prototypesFromSamples = new QAction("Add new prototype base from samples",prototypesMenu);
    prototypesMenu->addAction(prototypesFromSamples);
    projectView->addAction(prototypesFromSamples);
    connect(prototypesFromSamples,SIGNAL(triggered()),this,SLOT(on_actionAdd_prototypes_from_samples_triggered()));
    QAction *prototypesFromExpert = new QAction("Add new prototype base from expert",prototypesMenu);
    prototypesMenu->addAction(prototypesFromExpert);
    projectView->addAction(prototypesFromExpert);
    connect(prototypesFromExpert,SIGNAL(triggered()),this,SLOT(on_actionAdd_prototypes_from_expert_triggered()));
    QAction *prototypesFromMining = new QAction("Add new prototype base from data mining",prototypesMenu);
    prototypesMenu->addAction(prototypesFromMining);
    projectView->addAction(prototypesFromMining);
    connect(prototypesFromMining,SIGNAL(triggered()),this,SLOT(on_actionAdd_prototypes_from_Data_Mining_triggered()));
    QAction *importPrototypeBase = new QAction("Import prototype base from file",prototypesMenu);
    prototypesMenu->addAction(importPrototypeBase);
    projectView->addAction(importPrototypeBase);
    connect(importPrototypeBase,SIGNAL(triggered()),this,SLOT(onImportPrototypeBase()));
    // Covariates menu
    gisDataMenu = new QMenu(projectView);
    QAction *addGisData = new QAction("Add Covariates", gisDataMenu);
    gisDataMenu->addAction(addGisData);
    projectView->addAction(addGisData);
    connect(addGisData,SIGNAL(triggered()),this,SLOT(onAddGisData()));
    // gis layer menu
    gisLayerMenu = new QMenu(projectView);
    // delete this layer
    QAction *deleteGisLayer = new QAction("Delete this covariate", gisLayerMenu);
    gisLayerMenu->addAction(deleteGisLayer);
    projectView->addAction(deleteGisLayer);
    connect(deleteGisLayer,SIGNAL(triggered()),this,SLOT(onDeleteGisLayer()));
    // modify covariate name
    QAction *modifyCovName = new QAction("Modify covariate name", gisLayerMenu);
    gisLayerMenu->addAction(modifyCovName);
    projectView->addAction(modifyCovName);
    connect(modifyCovName,SIGNAL(triggered()),this,SLOT(onModifyCovName()));
    // modify covariate file
    QAction *modifyCovFile = new QAction("Modify covariate file", gisLayerMenu);
    gisLayerMenu->addAction(modifyCovFile);
    projectView->addAction(modifyCovFile);
    connect(modifyCovFile,SIGNAL(triggered()),this,SLOT(onModifyCovFile()));
    // prototype base menu
    prototypeBaseMenu = new QMenu(projectView);
    // rename prototype base
    QAction *renamePrototypeBase = new QAction("Rename prtotype base",prototypeBaseMenu);
    prototypeBaseMenu->addAction(renamePrototypeBase);
    projectView->addAction(renamePrototypeBase);
    connect(renamePrototypeBase,SIGNAL(triggered()),this,SLOT(onRenamePrototypeBase()));
    // add prototype from expert action
    addProtoExpert = new QAction("Add prototype from expert",prototypeBaseMenu);
    prototypeBaseMenu->addAction(addProtoExpert);
    projectView->addAction(addProtoExpert);
    connect(addProtoExpert,SIGNAL(triggered()),this,SLOT(onCreatePrototypeFromExpert()));
    // change covariate name action
    QAction *changeCovName = new QAction("Change covariate name",prototypeBaseMenu);
    prototypeBaseMenu->addAction(changeCovName);
    projectView->addAction(changeCovName);
    connect(changeCovName,SIGNAL(triggered()),this,SLOT(onChangeCovName()));
    // save prototypebase as xml action
    QAction *savePrototypeBase_xml = new QAction("Save as external .xml file",prototypeBaseMenu);
    prototypeBaseMenu->addAction(savePrototypeBase_xml);
    projectView->addAction(savePrototypeBase_xml);
    connect(savePrototypeBase_xml, SIGNAL(triggered()),this,SLOT(onSavePrototypeBase()));
    // export prototype base to .csv action
    QAction *exportPrototypeBase_csv = new QAction("Export to .csv file",prototypeBaseMenu);
    prototypeBaseMenu->addAction(exportPrototypeBase_csv);
    projectView->addAction(exportPrototypeBase_csv);
    connect(exportPrototypeBase_csv,SIGNAL(triggered()),this,SLOT(onExportPrototypeBase()));
    // delete prototype base action
    QAction *deletePrototypeBase = new QAction("Delete this prototype base",prototypeBaseMenu);
    prototypeBaseMenu->addAction(deletePrototypeBase);
    projectView->addAction(deletePrototypeBase);
    connect(deletePrototypeBase, SIGNAL(triggered()),this,SLOT(onDeletePrototypeBase()));
    // prototype menu
    prototypeMenu = new QMenu(projectView);
    // add rules action
    addRules = new QAction("Add rules to this prototype", prototypeMenu);
    prototypeMenu->addAction(addRules);
    projectView->addAction(addRules);
    connect(addRules,SIGNAL(triggered()),this,SLOT(onAddRules()));
    // delete prototype action
    QAction *deletePrototype = new QAction("Delete this prototype", prototypeMenu);
    prototypeMenu->addAction(deletePrototype);
    projectView->addAction(deletePrototype);
    connect(deletePrototype,SIGNAL(triggered()),this,SLOT(onDeletePrototype()));
    // prototype menu
    membershipMenu = new QMenu(projectView);
    // add rules action
    editRule = new QAction("Edit this function", prototypeMenu);
    membershipMenu->addAction(editRule);
    projectView->addAction(editRule);
    connect(editRule,SIGNAL(triggered()),this,SLOT(onEditRule()));
    saveRule = new QAction("Save Edit", prototypeMenu);
    membershipMenu->addAction(saveRule);
    projectView->addAction(saveRule);
    connect(saveRule,SIGNAL(triggered()),this,SLOT(saveEditRuleChange()));
    resetRule = new QAction("reset", prototypeMenu);
    membershipMenu->addAction(resetRule);
    projectView->addAction(resetRule);
    connect(resetRule,SIGNAL(triggered()),this,SLOT(resetEditRule()));
   projectViewInitialized = true;
}

void MainWindow::onEditRule(){
    if(myGraphicsView->membership->dataType==solim::CONTINUOUS){
        myGraphicsView->editFreehandRule = true;
        onAddFreehandPoint();
    } else
        myGraphicsView->editEnumRule = true;

}

void MainWindow::onAddFreehandPoint(){
    vector<double> *freeKnotX = new vector<double>;
    vector<double> *freeKnotY = new vector<double>;
    freeKnotX->clear();
    freeKnotX->shrink_to_fit();
    freeKnotY->clear();
    freeKnotY->shrink_to_fit();
    for(int i = 0; i<myGraphicsView->knotX.size();i++){
        freeKnotX->push_back(myGraphicsView->knotX[i]);
        freeKnotY->push_back(myGraphicsView->knotY[i]);
    }
    int sceneWidth = myGraphicsView->getScene()->width();
    int sceneHeight = myGraphicsView->getScene()->height();
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;
    QPen pen(Qt::black);
    pen.setWidth(1);
    if(freeKnotX->size()>2){
        // todo
        solim::Curve *c = new solim::Curve("tmp",solim::CONTINUOUS,freeKnotX,freeKnotY);
        c->range=(myGraphicsView->curveXMin - myGraphicsView->curveXMax);
        drawMembershipFunction(myGraphicsView->curveXMax,myGraphicsView->curveXMin,c);
        for(size_t i = 0; i<myGraphicsView->knotX.size();i++){
            double x = (myGraphicsView->knotX[i]-myGraphicsView->curveXMin)/(myGraphicsView->curveXMax-myGraphicsView->curveXMin);
            double y = myGraphicsView->knotY[i];
            myGraphicsView->getScene()->addRect(x*graphWidth+xStart-2,yEnd-y*graphHeight-2,4,4,pen,QBrush(Qt::black));
        }
        delete c;
    }
    else {
        solim::Curve *c = new solim::Curve();
        drawMembershipFunction(myGraphicsView->curveXMax,myGraphicsView->curveXMin,c);
        for(int i = 0; i<freeKnotX->size();i++){
            double x = (freeKnotX->at(i)-myGraphicsView->curveXMin)/(myGraphicsView->curveXMax-myGraphicsView->curveXMin);//0.5*(freeKnotX->at(i)+margin)/margin;
            double y = freeKnotY->at(i);
            myGraphicsView->getScene()->addRect(x*graphWidth+xStart-2,yEnd-y*graphHeight-2,4,4,pen,QBrush(Qt::black));
        }
    }
}

void MainWindow::saveEditRuleChange(){
    int protoPos = -1;
    int covPos = -1;
    for(size_t i = 0;i<proj->prototypes.size();i++){
        if(proj->prototypes[i].prototypeBaseName==currentBaseName
                && proj->prototypes[i].prototypeID==currentProtoName){
            protoPos = i;
            for(int j = 0; j<proj->prototypes[i].envConditionSize;j++){
                if(proj->prototypes[i].envConditions[j].covariateName==currentLayerName){
                    covPos = j;
                    break;
                }
            }
            break;
        }
    }
    proj->prototypes[protoPos].envConditions[covPos].changeCurve(myGraphicsView->membership);
    myGraphicsView->editFreehandRule = false;
    myGraphicsView->editEnumRule = false;
    drawMembershipFunction(myGraphicsView->curveXMax,myGraphicsView->curveXMin);
}

void MainWindow::resetEditRule(){
    drawMembershipFunction(myGraphicsView->curveXMax,myGraphicsView->curveXMin);
    myGraphicsView->editFreehandRule = false;
    myGraphicsView->editEnumRule = false;
}

void MainWindow::onAddEnumPoint(){
    myGraphicsView->getScene()->clear();
    vector<double> *freeKnotX = new vector<double>;
    vector<double> *freeKnotY = new vector<double>;
    freeKnotX->clear();
    freeKnotX->shrink_to_fit();
    freeKnotY->clear();
    freeKnotY->shrink_to_fit();
    for(int i = 0; i<myGraphicsView->knotX.size();i++){
        freeKnotX->push_back(myGraphicsView->knotX[i]);
        freeKnotY->push_back(myGraphicsView->knotY[i]);
    }
    solim::Curve *c = new solim::Curve("tmp",solim::CATEGORICAL,freeKnotX,freeKnotY);
    drawMembershipFunction(myGraphicsView->curveXMax,myGraphicsView->curveXMin,c);
    return;
}


void MainWindow::onExpanded(const QModelIndex& index)
{
    projectView->resizeColumnToContents(0);
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
            solim::Curve c = solim::Curve(covName, datatype, nodeNum, coords, range);
            c.typicalValue = atof(envAttri->FirstChildElement("TypicalValue")->GetText());
            proto.addConditions(c);
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
            on_actionSave_triggered();
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
            return false;
        default:
            on_actionSave_triggered();
        }
    }
    projectSaved = true;
    return true;
}

bool MainWindow::baseExistsWarning(string basename){
    for(size_t i = 0; i< proj->prototypeBaseNames.size();i++){
        if(basename==proj->prototypeBaseNames[i]){
            warn("Prototype base with the same base name already exists.");
            return true;
        }
    }
    return false;
}

void MainWindow::initParas(){
    bool validFile=false;
    QFile setting("./bin/setting.txt");
    if(setting.exists()){
        if(setting.open(QIODevice::ReadWrite)){
            QTextStream in(&setting);
            while(!in.atEnd()) {
                QString line = in.readLine();
                QStringList info = line.split("=");
                if(info.at(0)=="WorkingDir"){
                    QFileInfo dir=info.at(1);
                    if(dir.exists()&dir.isDir())
                        workingDir=info.at(1);
                    else
                        workingDir="./";
                    validFile=true;
                }
            }
        }
    }
    if(!validFile)
        workingDir="./";
}

void MainWindow::saveSetting(){
    QFile setting("./bin/setting.txt");
    if(setting.open(QIODevice::WriteOnly)){
        QTextStream out(&setting);
        out<<"WorkingDir="+workingDir<<endl;
    }
}

void MainWindow::on_actionAdd_Covariates_triggered() {
    if(proj==nullptr){
        QString studyArea = "";
        proj = new SoLIMProject();
        proj->projFilename = "./Untitled.slp";
        proj->projName = "Untitled";
        proj->studyArea="";
        proj->workingDir=workingDir;
        initModel();
        onAddGisData();
    }  else {
        onAddGisData();
    }
}

void MainWindow::on_actionResample_triggered()
{
    SoLIMProject *project = new SoLIMProject;
    project->workingDir = workingDir;
    SimpleDialog resample(SimpleDialog::RESAMPLE, project, this);
    resample.exec();
    QString inputFile = resample.filename;
    QString outputFile = resample.lineEdit2;
    QString referFile = resample.lineEdit3;
    if(inputFile.isEmpty()||outputFile.isEmpty()||referFile.isEmpty())
        return;
    BaseIO *inputLayer = new BaseIO(inputFile.toStdString());
    BaseIO *refLayer = new BaseIO(referFile.toStdString());
    bool consist = refLayer->compareIO(inputLayer);
    if(consist) {
        QMessageBox warning;
        warning.setText("The number of rows and columns, the projection, and the extent coordinates in two files are consistent, No need for resampling.");
        warning.exec();
        return;
    }
    else {
        if(inputLayer->resample(refLayer, outputFile.toStdString())){
            QMessageBox msg;
            msg.setText("Resampling succeeded!");
            msg.exec();
            return;
        } else {
            QMessageBox msg;
            msg.setText("Resampling failed! Please check if the input files are readable, and if the output file is writable.");
            msg.exec();
            return;
        }
    }
    delete inputLayer;
    workingDir = project->workingDir;
    delete project;
}

void MainWindow::on_actionValidation_triggered()
{
    if(proj==nullptr) return;
    Validation valid(proj, this);
    valid.exec();

}
