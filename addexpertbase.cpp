#include "addexpertbase.h"
#include "ui_addexpertbase.h"

AddExpertBase::AddExpertBase(SoLIMProject *proj, QWidget *parent) :
    QDialog(parent),proj(proj),
    ui(new Ui::AddExpertBase)
{
    ui->setupUi(this);
    myview = new MyGraphicsView();
    ui->layout_graphicsView->addWidget(myview);
    ui->lineEdit_prototype->setEnabled(false);
    ui->label_prototype->setEnabled(false);
    ui->btn_add_proto->setEnabled(false);
    ui->label_edit_proto->setEnabled(false);
    ui->comboBox_prototype->setEnabled(false);
    ui->label_add_rule->setEnabled(false);
    ui->radioButton_range->setChecked(true);
    ui->radioButton_range->setEnabled(false);
    ui->radioButton_point->setEnabled(false);
    ui->radioButton_freehand->setEnabled(false);
    ui->radioButton_enum->setEnabled(false);
    ui->label_cov->setEnabled(false);
    ui->comboBox_cov->setEnabled(false);
    QStringList gisLayers;
    for(int i = 0;i<proj->layernames.size();i++){
        gisLayers.append(proj->layernames[i].c_str());
    }
    gisLayers.append("[New covariate]");
    ui->comboBox_cov->addItems(gisLayers);
    ui->label_curve->setEnabled(false);
    ui->comboBox_curve->setEnabled(false);
    ui->label_range_cov->setVisible(false);
    ui->label_min_cov->setVisible(false);
    ui->lineEdit_min_cov->setVisible(false);
    ui->label_max_cov->setVisible(false);
    ui->lineEdit_max_cov->setVisible(false);
    ui->label_value1->setEnabled(false);
    ui->lineEdit_value1->setEnabled(false);
    ui->label_value2->setEnabled(false);
    ui->lineEdit_value2->setEnabled(false);
    ui->label_value3->setEnabled(false);
    ui->lineEdit_value3->setEnabled(false);
    ui->label_value4->setEnabled(false);
    ui->lineEdit_value4->setEnabled(false);
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(false);
    ui->btn_reset->setVisible(false);
    ui->btn_add_rule->setEnabled(false);
    ui->label_membership->setEnabled(false);
    //ui->graphicsView->setEnabled(false);
    ui->label_add_property->setEnabled(false);
    ui->label_prop_name->setEnabled(false);
    ui->lineEdit_prop_name->setEnabled(false);
    ui->label_prop_val->setEnabled(false);
    ui->lineEdit_prop_val->setEnabled(false);
    ui->checkBox_datatype_category->setEnabled(false);
    ui->btn_add_prop->setEnabled(false);
    ui->label_freehand_hint->setVisible(false);
    myview->editFreehandRule=false;
    connect(myview,SIGNAL(addFreehandPoint(const double, const double)),this,SLOT(onAddFreehandRule(const double, const double)));
    freeKnotX = new vector<double>;
    freeKnotY = new vector<double>;
}

AddExpertBase::~AddExpertBase()
{
    delete ui;
}

void AddExpertBase::on_btn_create_base_clicked()
{
    if(!ui->lineEdit_basename->text().isEmpty()){
        basename = ui->lineEdit_basename->text();
        emit createBase(basename);
        ui->btn_create_base->setEnabled(false);
        ui->lineEdit_basename->setEnabled(false);
        ui->label_prototype->setEnabled(true);
        ui->lineEdit_prototype->setEnabled(true);
        ui->btn_add_proto->setEnabled(true);
    }
}

void AddExpertBase::on_btn_add_proto_clicked()
{
    QString prototypeName = ui->lineEdit_prototype->text();
    if(prototypeNames.contains(prototypeName)) {
        QMessageBox warning;
        warning.setText("Prototype "+prototypeName+" already exists.");
        warning.exec();
        return;
    }
    emit createPrototype(basename, prototypeName);
    prototypeNames.append(prototypeName);
    ui->comboBox_prototype->addItems(prototypeNames);
    ui->comboBox_prototype->setCurrentText(ui->lineEdit_prototype->text());
    ui->comboBox_prototype->setEnabled(true);
    ui->label_edit_proto->setEnabled(true);
    ui->label_add_rule->setEnabled(true);
    ui->radioButton_range->setEnabled(true);
    ui->radioButton_point->setEnabled(true);
    ui->radioButton_freehand->setEnabled(true);
    ui->radioButton_enum->setEnabled(true);
    ui->label_cov->setEnabled(true);
    ui->comboBox_cov->setEnabled(true);
    ui->label_curve->setEnabled(true);
    ui->comboBox_curve->setEnabled(true);
    ui->label_value1->setEnabled(true);
    ui->lineEdit_value1->setEnabled(true);
    ui->label_value2->setEnabled(true);
    ui->lineEdit_value2->setEnabled(true);
    ui->label_value3->setEnabled(true);
    ui->lineEdit_value3->setEnabled(true);
    ui->label_value4->setEnabled(true);
    ui->lineEdit_value4->setEnabled(true);
    ui->btn_add_rule->setEnabled(true);
    ui->label_membership->setEnabled(true);
    //ui->graphicsView->setEnabled(true);
    ui->label_add_property->setEnabled(true);
    ui->label_prop_name->setEnabled(true);
    ui->lineEdit_prop_name->setEnabled(true);
    ui->label_prop_val->setEnabled(true);
    ui->lineEdit_prop_val->setEnabled(true);
    ui->checkBox_datatype_category->setEnabled(true);
    ui->btn_add_prop->setEnabled(true);
    myview->editFreehandRule=false;
    ui->label_freehand_hint->setVisible(false);
}

void AddExpertBase::on_radioButton_range_clicked()
{
    myview->getScene()->clear();
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(true);
    ui->comboBox_curve->setVisible(true);
    ui->label_value1->setVisible(true);
    ui->label_value1->setText("Low unity: ");
    ui->lineEdit_value1->setVisible(true);
    ui->label_value2->setVisible(true);
    ui->label_value2->setText("Low cross: ");
    ui->lineEdit_value2->setVisible(true);
    ui->label_value3->setVisible(true);
    ui->label_value3->setText("High unity:");
    ui->lineEdit_value3->setVisible(true);
    ui->label_value4->setVisible(true);
    ui->label_value4->setText("High cross:");
    ui->lineEdit_value4->setVisible(true);
    ui->lineEdit_value1->clear();
    ui->lineEdit_value2->clear();
    ui->lineEdit_value3->clear();
    ui->lineEdit_value4->clear();
    ui->label_range_cov->setVisible(false);
    ui->label_min_cov->setVisible(false);
    ui->lineEdit_min_cov->setVisible(false);
    ui->label_max_cov->setVisible(false);
    ui->lineEdit_max_cov->setVisible(false);
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(false);
    ui->btn_reset->setVisible(false);
    ui->label_freehand_hint->setVisible(false);
    myview->editFreehandRule=false;
}

void AddExpertBase::on_radioButton_point_clicked()
{
    myview->getScene()->clear();
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(false);
    ui->comboBox_curve->setVisible(false);
    ui->label_value1->setVisible(true);
    ui->label_value1->setText("Central X:  ");
    ui->lineEdit_value1->setVisible(true);
    ui->label_value2->setVisible(true);
    ui->label_value2->setText("Central Y:  ");
    ui->lineEdit_value2->setVisible(true);
    ui->label_value3->setVisible(true);
    ui->label_value3->setText("Left width: ");
    ui->lineEdit_value3->setVisible(true);
    ui->label_value4->setVisible(true);
    ui->label_value4->setText("Right width:");
    ui->lineEdit_value4->setVisible(true);
    ui->lineEdit_value1->clear();
    ui->lineEdit_value2->clear();
    ui->lineEdit_value3->clear();
    ui->lineEdit_value4->clear();
    ui->label_range_cov->setVisible(false);
    ui->label_min_cov->setVisible(false);
    ui->lineEdit_min_cov->setVisible(false);
    ui->label_max_cov->setVisible(false);
    ui->lineEdit_max_cov->setVisible(false);
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(false);
    ui->btn_reset->setVisible(false);
    ui->label_freehand_hint->setVisible(false);
    myview->editFreehandRule=false;
}

void AddExpertBase::on_radioButton_freehand_clicked()
{
    myview->getScene()->clear();
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(false);
    ui->comboBox_curve->setVisible(false);
    ui->label_value1->setVisible(false);
    ui->lineEdit_value1->setVisible(false);
    ui->label_value2->setVisible(false);
    ui->lineEdit_value2->setVisible(false);
    ui->label_value3->setVisible(false);
    ui->lineEdit_value3->setVisible(false);
    ui->label_value4->setVisible(false);
    ui->lineEdit_value4->setVisible(false);
    ui->label_range_cov->setVisible(true);
    ui->label_min_cov->setVisible(true);
    ui->lineEdit_min_cov->setVisible(true);
    ui->label_max_cov->setVisible(true);
    ui->lineEdit_max_cov->setVisible(true);
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(true);
    ui->btn_add_opt_val->setText("Start Editing");
    ui->btn_reset->setVisible(true);
    ui->lineEdit_max_cov->setText("");
    ui->lineEdit_min_cov->setText("");
    ui->label_freehand_hint->setVisible(false);
    qInfo()<<ui->comboBox_cov->maxVisibleItems()<<ui->comboBox_cov->currentIndex();
    if(ui->comboBox_cov->currentIndex()!=ui->comboBox_cov->maxVisibleItems()-1&&!ui->comboBox_cov->currentText().isEmpty()){
        for(int i=0;i<proj->layernames.size();i++){
            if(ui->comboBox_cov->currentText().toStdString()==proj->layernames[i]){
                BaseIO *lyr = new BaseIO(proj->filenames[i]);
                ui->lineEdit_max_cov->setText(QString::number(lyr->getDataMax()));
                ui->lineEdit_min_cov->setText(QString::number(lyr->getDataMin()));
                delete lyr;
            }
        }
    }
    enumViewInit=false;
    myview->editFreehandRule=false;
    freeKnotX->clear();
    freeKnotX->shrink_to_fit();
    freeKnotY->clear();
    freeKnotY->shrink_to_fit();
}

void AddExpertBase::on_radioButton_enum_clicked()
{
    myview->getScene()->clear();
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(false);
    ui->comboBox_curve->setVisible(false);
    ui->label_value1->setVisible(false);
    ui->lineEdit_value1->setVisible(false);
    ui->label_value2->setVisible(false);
    ui->lineEdit_value2->setVisible(false);
    ui->label_value3->setVisible(false);
    ui->lineEdit_value3->setVisible(false);
    ui->label_value4->setVisible(false);
    ui->lineEdit_value4->setVisible(false);
    ui->label_range_cov->setVisible(true);
    ui->label_min_cov->setVisible(true);
    ui->lineEdit_min_cov->setVisible(true);
    ui->label_max_cov->setVisible(true);
    ui->lineEdit_max_cov->setVisible(true);
    ui->lineEdit_opt_val->setVisible(true);
    ui->btn_add_opt_val->setVisible(true);
    ui->btn_add_opt_val->setText("Add Optimal Value");
    ui->btn_reset->setVisible(true);
    ui->lineEdit_max_cov->clear();
    ui->lineEdit_min_cov->clear();
    ui->lineEdit_opt_val->clear();
    ui->label_freehand_hint->setVisible(false);
    if(ui->comboBox_cov->currentIndex()!=ui->comboBox_cov->maxVisibleItems()-1&&!ui->comboBox_cov->currentText().isEmpty()){
        for(int i=0;i<proj->layernames.size();i++){
            if(ui->comboBox_cov->currentText().toStdString()==proj->layernames[i]){
                BaseIO *lyr = new BaseIO(proj->filenames[i]);
                ui->lineEdit_max_cov->setText(QString::number(lyr->getDataMax()));
                ui->lineEdit_min_cov->setText(QString::number(lyr->getDataMin()));
                delete lyr;
            }
        }
    }
    enumVals.clear();
    enumVals.shrink_to_fit();
    enumViewInit=false;
    myview->editFreehandRule=false;
}

void AddExpertBase::on_comboBox_cov_activated(const QString &arg1)
{
    myview->getScene()->clear();
    if(arg1=="[New covariate]"){
        AddGisDataDialog addGisData(this);
        addGisData.exec();
        if(addGisData.covariate.isEmpty()){
            ui->comboBox_cov->setCurrentIndex(0);
            return;
        }
        if(addGisData.filename.isEmpty()&&ui->radioButton_point->isChecked()){
            pointRuleWarn();
            return;
        }
        ui->comboBox_cov->insertItem(ui->comboBox_cov->count()-1,addGisData.covariate);
        ui->comboBox_cov->setCurrentIndex(ui->comboBox_cov->count()-2);
        if(!addGisData.filename.isEmpty()){
            proj->layernames.push_back(addGisData.covariate.toStdString());
            proj->filenames.push_back(addGisData.filename.toStdString());
            proj->layertypes.push_back(addGisData.datatype);
            emit addlayer();
        }
    }
    if(ui->radioButton_freehand->isChecked()||ui->radioButton_enum->isChecked()){
        for(int i = 0;i<proj->layernames.size();i++){
            if(proj->layernames[i]==ui->comboBox_cov->currentText().toStdString()){
                BaseIO *lyr = new BaseIO(proj->filenames[i]);
                ui->lineEdit_max_cov->setText(QString::number(lyr->getDataMax()));
                ui->lineEdit_min_cov->setText(QString::number(lyr->getDataMin()));
                delete lyr;
            }
        }
    }
    if(ui->radioButton_point->isChecked()){
        bool hasFileFlag = false;
        for(int i = 0;i<proj->layernames.size();i++){
            if(ui->comboBox_cov->currentText().toStdString()==proj->layernames[i]){
                hasFileFlag = true;
            }
        }
        if(!hasFileFlag){
            pointRuleWarn();
        }
    }
}

void AddExpertBase::pointRuleWarn(){
    QMessageBox warning;
    warning.setText("The chosen covariate does not have corresponding filename. Point rule cannot be set.");
    warning.exec();
    ui->comboBox_cov->setCurrentIndex(-1);
}
void AddExpertBase::on_btn_add_prop_clicked()
{
    QString propName=ui->lineEdit_prop_name->text();
    QString propVal = ui->lineEdit_prop_val->text();
    if(!propName.isEmpty()&&!propVal.isEmpty()){
        bool propValValid = true;
        double propVal_d=propVal.toDouble(&propValValid);
        if(!propValValid) return;
        for(int i = 0; i<proj->prototypes.size();i++){
            if(proj->prototypes[i].prototypeBaseName==basename.toStdString()
                    &&proj->prototypes[i].prototypeID==ui->comboBox_prototype->currentText().toStdString()){
                if(ui->checkBox_datatype_category->isChecked())
                    proj->prototypes[i].addProperties(propName.toStdString(),propVal_d,solim::CATEGORICAL);
                else
                    proj->prototypes[i].addProperties(propName.toStdString(),propVal_d,solim::CONTINUOUS);
                addSuccess("Property");
                return;
            }
        }
    }
}

void AddExpertBase::addSuccess(QString content){
    QMessageBox propertyAdded;
    propertyAdded.setText(content+" added to prototype success!");
    propertyAdded.show();
    propertyAdded.button(QMessageBox::Ok)->animateClick(5000);
    emit updatePrototype();
}

void AddExpertBase::on_btn_add_rule_clicked()
{
    // check valid cov name
    string covname=ui->comboBox_cov->currentText().toStdString();
    if(covname=="[New covariate]"){
        QMessageBox warn;
        warn.setText("Please specify covariate.");
        warn.exec();
        return;
    }
    // check valid cov type
    string filename="";
    for(int i = 0;i<proj->filenames.size();i++){
        if(covname==proj->layernames[i]){
            filename=proj->filenames[i];
            if(proj->layertypes[i]=="CATEGORICAL"){
                if(!ui->radioButton_enum->isChecked()){
                    QMessageBox warn;
                    warn.setText("Categorical covariate can only use enumerated rule.");
                    warn.exec();
                    return;
                }
            }
        }
    }
    // check valid prototype
    int pos=0;
    while(pos<proj->prototypes.size()){
        if(proj->prototypes[pos].prototypeBaseName==basename.toStdString()
                &&proj->prototypes[pos].prototypeID==ui->comboBox_prototype->currentText().toStdString()){
            break;
        }
        ++pos;
    }
    if(pos==proj->prototypes.size()) return;
    // check unique cov name
    for(int i=0;i<proj->prototypes[pos].envConditions.size();i++){
        if(covname==proj->prototypes[pos].envConditions[i].covariateName){
            QMessageBox warn;
            warn.setText("Rule for covariate \""+ui->comboBox_cov->currentText()+"\" has been added.");
            warn.exec();
            return;
        }
    }
    if(ui->radioButton_range->isChecked()){
        solim::CurveTypeEnum type;
        double lu,lc,hu,hc;
        if(ui->comboBox_curve->currentText()=="Bell-shaped"){
            bool*toNumFlag = new bool;
            lu=ui->lineEdit_value1->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            lc=ui->lineEdit_value2->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            hu=ui->lineEdit_value3->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            hc=ui->lineEdit_value4->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            type = solim::BELL_SHAPED;
        } else if(ui->comboBox_curve->currentText()=="S-shaped"){
            bool*toNumFlag = new bool;
            lu=ui->lineEdit_value1->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            lc=ui->lineEdit_value2->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            hu=-1;
            hc=-1;
            type=solim::S_SHAPED;
        } else if(ui->comboBox_curve->currentText()=="Z-shaped"){
            bool*toNumFlag = new bool;
            hu=ui->lineEdit_value3->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            hc=ui->lineEdit_value4->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            lu=-1;
            lc=-1;
            type=solim::Z_SHAPED;
        }
        solim::Curve c=solim::Curve(covname,lu,hu,lc,hc,type);
        proj->prototypes[pos].envConditions.push_back(c);
        proj->prototypes[pos].envConditionSize++;
        drawMembershipFunction(&c);
        addSuccess("Rule");
        emit updatePrototype();
    } else if(ui->radioButton_point->isChecked()){
        if(!filename.empty()){
            bool*toNumFlag = new bool;
            double centralX=ui->lineEdit_value1->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            double centralY=ui->lineEdit_value2->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            double leftWidth=ui->lineEdit_value3->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            double rightWidth=ui->lineEdit_value4->text().toDouble(toNumFlag);
            if(!*toNumFlag) return;
            BaseIO *lyr=new BaseIO(filename);
            int row,col;
            float *unity=new float;
            lyr->geoToGlobalXY(centralX,centralY,col,row);
            lyr->read(col,row,1,1,unity);
            solim::Curve c=solim::Curve(covname,*unity,*unity,*unity-leftWidth,*unity+rightWidth,solim::BELL_SHAPED);
            proj->prototypes[pos].envConditions.push_back(c);
            proj->prototypes[pos].envConditionSize++;
            drawMembershipFunction(&c);
            addSuccess("Rule");
            emit updatePrototype();
        } else{
            pointRuleWarn();
        }

    } else if(ui->radioButton_freehand->isChecked()){
        if(freeKnotX->size()>2){
            solim::Curve c = solim::Curve(ui->comboBox_cov->currentText().toStdString(),solim::CONTINUOUS,freeKnotX,freeKnotY);
            c.range=fabs(enumMax)>fabs(enumMin)?fabs(enumMax):fabs(enumMin);
            proj->prototypes[pos].envConditions.push_back(c);
            proj->prototypes[pos].envConditionSize++;
            addSuccess("Rule");
            emit updatePrototype();
            freeKnotX->clear();
            freeKnotX->shrink_to_fit();
            freeKnotY->clear();
            freeKnotY->shrink_to_fit();
            myview->editFreehandRule=false;
        }
    } else if(ui->radioButton_enum->isChecked()){
        if(enumVals.size()>0){
            solim::Curve c=solim::Curve(covname,solim::CATEGORICAL);
            for(int i = 0;i<enumVals.size();i++){
                c.addKnot(enumVals[i],1);
            }
            c.typicalValue=enumVals[0];
            c.range=fabs(enumMax)>fabs(enumMin)?fabs(enumMax):fabs(enumMin);
            proj->prototypes[pos].envConditions.push_back(c);
            proj->prototypes[pos].envConditionSize++;
            addSuccess("Rule");
            emit updatePrototype();
        }
    }
}

void AddExpertBase::on_comboBox_curve_activated(const QString &arg1) {
    if(arg1=="Bell-shaped"){
        ui->lineEdit_value1->setEnabled(true);
        ui->lineEdit_value2->setEnabled(true);
        ui->lineEdit_value3->setEnabled(true);
        ui->lineEdit_value4->setEnabled(true);
    } else if(arg1=="S-shaped"){
        ui->lineEdit_value1->setEnabled(true);
        ui->lineEdit_value2->setEnabled(true);
        ui->lineEdit_value3->setEnabled(false);
        ui->lineEdit_value4->setEnabled(false);
    } else if(arg1=="Z-shaped"){
        ui->lineEdit_value1->setEnabled(false);
        ui->lineEdit_value2->setEnabled(false);
        ui->lineEdit_value3->setEnabled(true);
        ui->lineEdit_value4->setEnabled(true);
    }
}

void AddExpertBase::drawMembershipFunction(solim::Curve *c) {
    QGraphicsScene *scene = myview->getScene();
    scene->clear();
    scene->setSceneRect(0,0,myview->width()*0.9,myview->height()*0.9);
    QPen curvePen(Qt::black);
    QPen axisPen(Qt::blue);
    axisPen.setWidth(2);
    curvePen.setWidth(1);
    int sceneWidth = scene->width();
    int sceneHeight = scene->height();
    double scale = c->range;
    int margin;
    if(ui->radioButton_freehand->isChecked()){
        margin=scale;
        scale = 2*scale;
    }else{
        if(scale<10) scale = (int(scale)+1)*2;
        else scale = (int(scale/10)+1)*10*2;
        margin = 0.5*scale;
    }

    // set axis
    scene->addLine(0.05*sceneWidth,0.85*sceneHeight,0.85*sceneWidth,0.85*sceneHeight,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight+1,0.85*sceneWidth-3,0.85*sceneHeight+4,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight,0.85*sceneWidth-3,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.45*sceneWidth,0.85*sceneHeight,0.45*sceneWidth,0.1*sceneHeight,axisPen);
    scene->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth-3,0.1*sceneHeight+3,axisPen);
    scene->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth+3,0.1*sceneHeight+3,axisPen);
    // set label
    scene->addLine(0.80*sceneWidth,0.85*sceneHeight,0.80*sceneWidth,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.45*sceneWidth,0.15*sceneHeight,0.45*sceneWidth+3,0.15*sceneHeight,axisPen);

    // set axis names
    QGraphicsTextItem *yaxisName = scene->addText("Optimality value");
    yaxisName->setFont(QFont("Times", 10, QFont::Bold));
    yaxisName->setPos(0.45*sceneWidth, 0.1*sceneHeight-20);
    QGraphicsTextItem *xaxisName = scene->addText("Covariate value");
    xaxisName->setFont(QFont("Times", 10, QFont::Bold));
    xaxisName->setPos(0.85*sceneWidth, 0.85*sceneHeight-10);

    QGraphicsTextItem *yaxis1 = scene->addText("1");
    yaxis1->setFont(QFont("Times", 10, QFont::Bold));
    yaxis1->setPos(0.45*sceneWidth-15,0.15*sceneHeight-10);
    QGraphicsTextItem *yaxis0 = scene->addText("0");
    yaxis0->setFont(QFont("Times", 10, QFont::Bold));
    yaxis0->setPos(0.45*sceneWidth-5,0.85*sceneHeight);
    QGraphicsTextItem *xaxis1 = scene->addText(QString::number(margin));
    xaxis1->setFont(QFont("Times", 10, QFont::Bold));
    xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
    QGraphicsTextItem *xaxis0 = scene->addText(QString::number(-margin));
    xaxis0->setFont(QFont("Times", 10, QFont::Bold));
    xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
    double previousx,previousy;
    previousy = c->getOptimality(0-margin);
    previousx = -margin;

    double x,y;
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;
    for(int i =0;i<100;i++){
        x =i*2*margin/101.0-margin;
        y = c->getOptimality(x);
        if(fabs(y+1)<VERY_SMALL ||fabs(previousy+1)<VERY_SMALL)
            continue;
        scene->addLine((previousx+margin)/scale*graphWidth+xStart,yEnd-previousy*graphHeight,(x+margin)/scale*graphWidth+xStart,yEnd-y*graphHeight,curvePen);
        previousx = x;
        previousy = y;
    }
}

void AddExpertBase::on_btn_add_opt_val_clicked() {
    if(!enumViewInit){
        if(!ui->lineEdit_max_cov->text().isEmpty()&&!ui->lineEdit_min_cov->text().isEmpty()){
            bool*toNumFlag = new bool;
            enumMax=ui->lineEdit_max_cov->text().toInt(toNumFlag);
            if(!*toNumFlag) { enumRuleWarn();  return; }
            enumMin=ui->lineEdit_min_cov->text().toInt(toNumFlag);
            if(!*toNumFlag) { enumRuleWarn();  return; }
            if(enumMax<enumMin) { enumRuleWarn();  return; }
            myview->getScene()->clear();
            drawEnumRange();
            enumViewInit=true;
        }
    }
    if(ui->radioButton_freehand->isChecked()){
        ui->label_freehand_hint->setVisible(true);
        myview->editFreehandRule=true;
    }
    if(ui->radioButton_enum->isChecked()){
        bool*toNumFlag = new bool;
        int num=ui->lineEdit_opt_val->text().toInt(toNumFlag);
        if(*toNumFlag){
            if(num>enumMax || num<enumMin){
                QMessageBox warn;
                warn.setText("Optimal value should be in the range of covariate value");
                warn.exec();
                return;
            }
            for(int i=0;i<enumVals.size();i++){
                if(num==enumVals[i])
                    return;
            }
            enumVals.push_back(num);
            drawEnum(num);
        }
    }
}

void AddExpertBase::drawEnumRange(){
    int margin = fabs(enumMax)>fabs(enumMin)?fabs(enumMax):fabs(enumMin);
    QGraphicsScene *scene = myview->getScene();
    scene->clear();
    scene->setSceneRect(0,0,myview->width()*0.9,myview->height()*0.9);
    QPen curvePen(Qt::black);
    QPen axisPen(Qt::blue);
    axisPen.setWidth(2);
    curvePen.setWidth(1);
    int sceneWidth = scene->width();
    int sceneHeight = scene->height();
    scene->addLine(0.05*sceneWidth,0.85*sceneHeight,0.85*sceneWidth,0.85*sceneHeight,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight+1,0.85*sceneWidth-3,0.85*sceneHeight+4,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight,0.85*sceneWidth-3,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.80*sceneWidth,0.85*sceneHeight,0.80*sceneWidth,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.85*sceneHeight-3,axisPen);
    // set axis names
    QGraphicsTextItem *yaxisName = scene->addText("Optimality value");
    yaxisName->setFont(QFont("Times", 10, QFont::Bold));
    yaxisName->setPos(0.45*sceneWidth, 0.1*sceneHeight-20);
    QGraphicsTextItem *xaxisName = scene->addText("Covariate value");
    xaxisName->setFont(QFont("Times", 10, QFont::Bold));
    xaxisName->setPos(0.85*sceneWidth, 0.85*sceneHeight-10);

    QGraphicsTextItem *yaxis1 = scene->addText("1");
    yaxis1->setFont(QFont("Times", 10, QFont::Bold));
    yaxis1->setPos(0.45*sceneWidth-15,0.15*sceneHeight-10);
    QGraphicsTextItem *yaxis0 = scene->addText("0");
    yaxis0->setFont(QFont("Times", 10, QFont::Bold));
    yaxis0->setPos(0.45*sceneWidth-5,0.85*sceneHeight);

    if(ui->radioButton_freehand->isChecked()||enumMax*enumMin<0||!enumMax>0){
        // set axis
        scene->addLine(0.45*sceneWidth,0.85*sceneHeight,0.45*sceneWidth,0.1*sceneHeight,axisPen);
        scene->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth-3,0.1*sceneHeight+3,axisPen);
        scene->addLine(0.45*sceneWidth,0.1*sceneHeight,0.45*sceneWidth+3,0.1*sceneHeight+3,axisPen);
        // set label
        scene->addLine(0.45*sceneWidth,0.15*sceneHeight,0.45*sceneWidth+3,0.15*sceneHeight,axisPen);
        QGraphicsTextItem *xaxis1 = scene->addText(QString::number(margin));
        xaxis1->setFont(QFont("Times", 10, QFont::Bold));
        xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
        QGraphicsTextItem *xaxis0 = scene->addText(QString::number(-margin));
        xaxis0->setFont(QFont("Times", 10, QFont::Bold));
        xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);

    }
    else if(enumMax>0){
        scene->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.1*sceneHeight,axisPen);
        scene->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth-3,0.1*sceneHeight+3,axisPen);
        scene->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth+3,0.1*sceneHeight+3,axisPen);
        // set label
        scene->addLine(0.10*sceneWidth,0.15*sceneHeight,0.10*sceneWidth+3,0.15*sceneHeight,axisPen);
        yaxis1->setPos(0.10*sceneWidth-15,0.15*sceneHeight-10);
        yaxis0->setPos(0.10*sceneWidth-5,0.85*sceneHeight);
        QGraphicsTextItem *xaxis1 = scene->addText(QString::number(enumMax));
        xaxis1->setFont(QFont("Times", 10, QFont::Bold));
        xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
        QGraphicsTextItem *xaxis0 = scene->addText(QString::number(0));
        xaxis0->setFont(QFont("Times", 10, QFont::Bold));
        xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
        yaxisName->setPos(0.10*sceneWidth, 0.1*sceneHeight-20);
    }
}
void AddExpertBase::drawEnum(int num){
    int margin = fabs(enumMax)>fabs(enumMin)?fabs(enumMax):fabs(enumMin);
    QGraphicsScene *scene = myview->getScene();
    int xStart = 0.10*scene->width();
    int graphWidth = 0.7*scene->width();
    QPen curvePen(Qt::black);
    curvePen.setWidth(2);
    if(enumMax*enumMin<0||!enumMax>0){
        scene->addLine(0.5*(num+margin)/margin*graphWidth+xStart,0.85*scene->height(),0.5*(num+margin)/margin*graphWidth+xStart,0.15*scene->height(),curvePen);
        QGraphicsTextItem *tag = scene->addText(QString::number(num));
        tag->setFont(QFont("Times", 8));
        tag->setDefaultTextColor(Qt::blue);
        tag->setPos(0.5*(num+margin)/margin*graphWidth+xStart-4*tag->toPlainText().size(),0.85*scene->height());
    }else{
        scene->addLine(1.0*num/enumMax*graphWidth+xStart,0.85*scene->height(),1.0*num/enumMax*graphWidth+xStart,0.15*scene->height(),curvePen);
        QGraphicsTextItem *tag = scene->addText(QString::number(num));
        tag->setFont(QFont("Times", 8));
        tag->setDefaultTextColor(Qt::blue);
        tag->setPos(1.0*num/enumMax*graphWidth+xStart-4*tag->toPlainText().size(),0.85*scene->height());
    }
}

void AddExpertBase::enumRuleWarn(){
    QMessageBox warning;
    warning.setText("Please input valid maximum and minimun value.");
    warning.exec();
}

void AddExpertBase::on_btn_reset_clicked()
{
    if(ui->radioButton_enum->isChecked()){
        enumVals.clear();
        enumVals.shrink_to_fit();
        myview->getScene()->clear();
    } else if(ui->radioButton_freehand->isChecked()){
        freeKnotX->clear();
        freeKnotX->shrink_to_fit();
        freeKnotY->clear();
        freeKnotY->shrink_to_fit();
        myview->getScene()->clear();
    }
}

void AddExpertBase::onAddFreehandRule(const double x, const double y){
    int margin = fabs(enumMax)>fabs(enumMin)?fabs(enumMax):fabs(enumMin);
    double knotX = x*2*margin-margin;
    freeKnotX->push_back(knotX);
    freeKnotY->push_back(y);
    qInfo()<<knotX<<y;
    if(freeKnotX->size()>2){
        solim::Curve *c = new solim::Curve(ui->comboBox_cov->currentText().toStdString(),solim::CONTINUOUS,freeKnotX,freeKnotY);
        c->range=margin;
        drawMembershipFunction(c);
        int sceneWidth = myview->getScene()->width();
        int sceneHeight = myview->getScene()->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        QPen pen(Qt::black);
        pen.setWidth(1);
        for(int i = 0; i<freeKnotX->size();i++){
            double x=0.5*(freeKnotX->at(i)+margin)/margin;
            double y = freeKnotY->at(i);
            myview->getScene()->addLine(x*graphWidth+xStart-2,yEnd-y*graphHeight-2,x*graphWidth+xStart+2,yEnd-y*graphHeight+2,pen);
            myview->getScene()->addLine(x*graphWidth+xStart-2,yEnd-y*graphHeight+2,x*graphWidth+xStart+2,yEnd-y*graphHeight-2,pen);
        }
        delete c;
    }
}
