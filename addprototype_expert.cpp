#include "addprototype_expert.h"
#include "ui_addprototype_expert.h"

AddPrototype_Expert::AddPrototype_Expert(SoLIMProject *proj, int protoNum, string currentBaseName, QWidget *parent) :
    QDialog(parent),proj(proj),protoNum(protoNum),currentBasename(currentBaseName),
    ui(new Ui::AddPrototype_Expert)
{
    ui->setupUi(this);
    myview = new MyGraphicsView();
    myview->setFixedHeight(width()*0.6);
    ui->layout_graphicsView->addWidget(myview);
    myview->setVisible(false);
    ui->label_freehand_hint->setTextFormat(Qt::RichText);
    ui->label_freehand_hint->setStyleSheet("QLabel { background-color : lightGray; color : black; }");
    ui->label_freehand_hint->setFrameStyle(QFrame::Box);
    ui->label_freehand_hint->setWordWrap(true);
//    ui->label_prototype->setTextFormat(Qt::RichText);
//    string prototypeInfo="Creation of Rules for prototype <b>"+proj->prototypes[protoNum].prototypeID
//            +"</b> from prototype base <b>"+proj->prototypes[protoNum].prototypeBaseName+"</b>";
//    ui->label_prototype->setText(prototypeInfo.c_str());
    QStringList gisLayers;
    for(int i = 0;i<proj->layernames.size();i++){
        gisLayers.append(proj->layernames[i].c_str());
    }
    gisLayers.append("[New covariate]");
    QFont f = ui->label_add_property->font();
    f.setBold(true);
    f.setPointSize(f.pointSize()+1);
    ui->label_add_property->setFont(f);
    ui->label_add_rule->setFont(f);
    ui->label_add_rule->setWordWrap(true);
    ui->comboBox_cov->addItems(gisLayers);
    ui->comboBox_cov->setCurrentIndex(-1);
    ui->comboBox_cov->setEditable(false);
    ui->comboBox_datatype->setCurrentIndex(-1);
    ui->comboBox_datatype->setEditable(false);
    ui->label_curve->setVisible(false);
    ui->comboBox_curve->setVisible(false);
    ui->comboBox_curve->setEditable(false);
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(false);
    ui->btn_reset->setVisible(false);
    ui->btn_add_rule->setVisible(false);
    ui->label_freehand_hint->setVisible(false);
    myview->editFreehandRule=false;
    myview->editEnumRule=false;
    ui->label_lc->setVisible(false);
    ui->lineEdit_lc->setVisible(false);
    ui->label_lu->setVisible(false);
    ui->lineEdit_lu->setVisible(false);
    ui->label_hu->setVisible(false);
    ui->lineEdit_hu->setVisible(false);
    ui->label_hc->setVisible(false);
    ui->lineEdit_hc->setVisible(false);
    ui->label_membership->setVisible(false);
    connect(myview,SIGNAL(addFreehandPoint()),this,SLOT(onAddFreehandPoint()));
    connect(myview,SIGNAL(addEnumPoint()),this,SLOT(onAddEnumPoint()));
    freeKnotX = new vector<double>;
    freeKnotY = new vector<double>;
    if(protoNum<0){
        ui->btn_add_prop->setEnabled(false);
        ui->comboBox_cov->setEnabled(false);
        ui->radioButton_range->setEnabled(false);
        ui->radioButton_freehand->setEnabled(false);
        ui->radioButton_enum->setEnabled(false);
    } else {
        ui->lineEdit_prototype->setText(proj->prototypes[protoNum].prototypeID.c_str());
        ui->lineEdit_prototype->setReadOnly(true);
        ui->btn_create->setVisible(false);
        setWindowTitle("Add rules to prototype");
    }
    ui->radioButton_enum->setEnabled(false);
    ui->radioButton_freehand->setEnabled(false);
    ui->radioButton_range->setEnabled(false);
}

AddPrototype_Expert::~AddPrototype_Expert()
{
    delete ui;
}

void AddPrototype_Expert::on_radioButton_range_toggled(bool checked)
{
    if(!checked) return;
    myview->getScene()->clear();
    myview->knotX.clear();
    myview->knotY.clear();
    myview->knotX.shrink_to_fit();
    myview->knotY.shrink_to_fit();
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(true);
    ui->comboBox_curve->setVisible(true);
    ui->label_lc->setVisible(true);
    ui->lineEdit_lc->setVisible(true);
    ui->label_lu->setVisible(true);
    ui->lineEdit_lu->setVisible(true);
    ui->label_hu->setVisible(true);
    ui->lineEdit_hu->setVisible(true);
    ui->label_hc->setVisible(true);
    ui->lineEdit_hc->setVisible(true);
    ui->lineEdit_lc->clear();
    ui->lineEdit_lu->clear();
    ui->lineEdit_hu->clear();
    ui->lineEdit_hc->clear();
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(false);
    ui->btn_reset->setText("Preview");
    ui->btn_reset->setVisible(true);
    ui->btn_add_rule->setVisible(true);
    ui->label_freehand_hint->setVisible(true);
    myview->editFreehandRule=false;
    myview->editEnumRule=false;
    ui->label_freehand_hint->setText("<b>Hint</b>: <i><b>Low cross/high cross</b></i>: The covariable values when optimality value decreased to 0.5.<br>"
                                     "<i><b>Low unity/high unity</b></i>: The covariable values when optimality value is 1.");
}

void AddPrototype_Expert::on_radioButton_freehand_toggled(bool checked)
{
    if(!checked) return;
    myview->knotX.clear();
    myview->knotY.clear();
    myview->knotX.shrink_to_fit();
    myview->knotY.shrink_to_fit();
    on_lineEdit_max_cov_textChanged(ui->lineEdit_max_cov->text());
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(false);
    ui->comboBox_curve->setVisible(false);
    ui->label_lc->setVisible(false);
    ui->lineEdit_lc->setVisible(false);
    ui->label_lu->setVisible(false);
    ui->lineEdit_lu->setVisible(false);
    ui->label_hu->setVisible(false);
    ui->lineEdit_hu->setVisible(false);
    ui->label_hc->setVisible(false);
    ui->lineEdit_hc->setVisible(false);
    ui->lineEdit_opt_val->setVisible(false);
    ui->btn_add_opt_val->setVisible(false);
    ui->btn_reset->setVisible(true);
    ui->btn_reset->setText("Reset");
    ui->btn_add_rule->setVisible(true);
    ui->label_freehand_hint->setVisible(true);
    enumViewInit=false;
    freeKnotX->clear();
    freeKnotX->shrink_to_fit();
    freeKnotY->clear();
    freeKnotY->shrink_to_fit();
    ui->label_freehand_hint->setText("<b>Hint</b>: Double click membership function view to add or delete points. Move points to adjust spline");
    myview->editEnumRule=false;
    myview->editFreehandRule=true;
}

void AddPrototype_Expert::on_radioButton_enum_toggled(bool checked)
{
    if(!checked) return;
    myview->knotX.clear();
    myview->knotY.clear();
    myview->knotX.shrink_to_fit();
    myview->knotY.shrink_to_fit();
    on_lineEdit_max_cov_textChanged(ui->lineEdit_max_cov->text());
    ui->label_cov->setVisible(true);
    ui->comboBox_cov->setVisible(true);
    ui->label_curve->setVisible(false);
    ui->comboBox_curve->setVisible(false);
    ui->label_lc->setVisible(false);
    ui->lineEdit_lc->setVisible(false);
    ui->label_lu->setVisible(false);
    ui->lineEdit_lu->setVisible(false);
    ui->label_hu->setVisible(false);
    ui->lineEdit_hu->setVisible(false);
    ui->label_hc->setVisible(false);
    ui->lineEdit_hc->setVisible(false);
    ui->lineEdit_opt_val->setVisible(true);
    ui->btn_add_opt_val->setVisible(true);
    ui->btn_reset->setVisible(true);
    ui->btn_reset->setText("Reset");
    ui->btn_add_rule->setVisible(true);
    ui->lineEdit_opt_val->clear();
    ui->label_freehand_hint->setVisible(true);
    enumVals.clear();
    enumVals.shrink_to_fit();
    enumViewInit=false;
    myview->editFreehandRule=false;
    myview->editEnumRule=true;
    ui->label_freehand_hint->setText("<b>Hint</b>: Enumerated rules are used for categorical environmental variables, e.g. geology, land use type. \
Add all possible conditions when prototype occurs as optimality value");
}

void AddPrototype_Expert::on_comboBox_cov_activated(const QString &arg1)
{
    ui->lineEdit_max_cov->clear();
    ui->lineEdit_min_cov->clear();
    myview->getScene()->clear();
    if(arg1=="[New covariate]"){
        SimpleDialog addGisData(SimpleDialog::ADDCOVARIATE,proj,this);
        addGisData.exec();
        if(addGisData.covariate.isEmpty()){
            ui->comboBox_cov->setCurrentIndex(0);
            return;
        }
        if(proj->addLayer(addGisData.covariate.toStdString(),addGisData.datatype,addGisData.filename.toStdString())){
            ui->comboBox_cov->insertItem(ui->comboBox_cov->count()-1,addGisData.covariate);
            ui->comboBox_cov->setCurrentIndex(ui->comboBox_cov->count()-2);
        } else{
            for(int i = 0;i<ui->comboBox_cov->count();i++){
                if(ui->comboBox_cov->itemText(i)==addGisData.covariate)
                    ui->comboBox_cov->setCurrentIndex(i);
            }
        }
        ui->radioButton_enum->setChecked(false);
        ui->radioButton_freehand->setChecked(false);
        ui->radioButton_range->setChecked(false);
    }
    for(int i = 0;i<proj->layernames.size();i++){
        if(proj->layernames[i]==ui->comboBox_cov->currentText().toStdString()){
            ui->lineEdit_filename->setText(proj->filenames[i].c_str());
            if(proj->layertypes[i]=="CONTINUOUS")   ui->comboBox_datatype->setCurrentIndex(0);
            else ui->comboBox_datatype->setCurrentIndex(1);
            ui->lineEdit_filename->setReadOnly(false);
            ui->lineEdit_max_cov->setReadOnly(false);
            ui->lineEdit_min_cov->setReadOnly(false);
            ui->comboBox_datatype->setEnabled(true);
            if(QFileInfo(proj->filenames[i].c_str()).exists()){
                BaseIO *lyr = new BaseIO(proj->filenames[i]);
                if(lyr->isOpened()){
                    ui->lineEdit_max_cov->setText(QString::number(lyr->getDataMax()));
                    ui->lineEdit_min_cov->setText(QString::number(lyr->getDataMin()));
                    rangeMax = lyr->getDataMax();
                    rangeMin = lyr->getDataMin();
                    ui->lineEdit_filename->setReadOnly(true);
                    ui->lineEdit_max_cov->setReadOnly(true);
                    ui->lineEdit_min_cov->setReadOnly(true);
                    ui->comboBox_datatype->setDisabled(true);
                }
                delete lyr;
            }
            if(ui->radioButton_range->isChecked())
                on_radioButton_range_toggled(true);
            if(ui->radioButton_freehand->isChecked())
                on_radioButton_freehand_toggled(true);
            if(ui->radioButton_enum->isChecked())
                on_radioButton_enum_toggled(true);
            return;
        }
    }
    if(ui->lineEdit_max_cov->text().isEmpty()||ui->lineEdit_min_cov->text().isEmpty()){
        ui->label_freehand_hint->setText("<b>Hint</b>: Specify the Maximum and Minimum values of covariate.");
    }
}

void AddPrototype_Expert::on_btn_add_prop_clicked()
{
    QString propName=ui->lineEdit_prop_name->text();
    QString propVal = ui->lineEdit_prop_val->text();
    solim::DataTypeEnum propType = solim::CONTINUOUS;
    if(ui->checkBox_datatype_category->isChecked()) propType = solim::CATEGORICAL;
    if(!propName.isEmpty()&&!propVal.isEmpty()){
        bool propValValid = true;
        double propVal_d=propVal.toDouble(&propValValid);
        if(!propValValid) return;
        for(int i = 0; i < proj->prototypes.size();i++){
            bool break_flag = false;
            if (proj->prototypes[i].prototypeBaseName==currentBasename){
                for(int j = 0; j < proj->prototypes[i].properties.size(); j++){
                    if(proj->prototypes[i].properties[j].propertyName==propName.toStdString()){
                        if (propType!=proj->prototypes[i].properties[j].soilPropertyType){
                            warn("Property datatype contradicts with existing records in the prototype base. The datatype (checkbox) selection has been changed. Please try again.");
                            ui->checkBox_datatype_category->toggle();
                            return;
                        }
                        break_flag = true;
                        break;
                    }
                }
                if(break_flag) break;
            }
        }
        for(int i = 0; i<proj->prototypes[protoNum].properties.size();i++){
            if(proj->prototypes[protoNum].properties[i].propertyName==propName.toStdString()){
                warn("This property has been add before. The value will be updated.");
            }
        }
        proj->prototypes[protoNum].addProperties(propName.toStdString(),propVal_d,propType);
        emit updatePrototype();
        ui->lineEdit_prop_name->clear();
        ui->lineEdit_prop_val->clear();
        return;
    }
}

void AddPrototype_Expert::on_btn_add_rule_clicked()
{
    // check valid cov name
    string covname=ui->comboBox_cov->currentText().toStdString();
    if(covname=="[New covariate]"){
        warn("Please specify covariate.");
        return;
    }
    // check unique cov name
    for(int i=0;i<proj->prototypes[protoNum].envConditionSize;i++){
        if(covname==proj->prototypes[protoNum].envConditions[i].covariateName){
            warn("Rule for covariate \""+ui->comboBox_cov->currentText()+"\" has been added.");
            return;
        }
    }
    if(ui->radioButton_range->isChecked()){
        solim::Curve c;
        bool warn;
        if(getPointRule(c,warn)){
            if(warn){
                QMessageBox warn;
                warn.setText("The ranges of the rule is inconsistent with the range of the covariate. Are you sure to add this rule?");
                warn.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                warn.setDefaultButton(QMessageBox::Yes);
                int ret = warn.exec();
                switch(ret){
                case QMessageBox::Cancel:
                    return;
                case QMessageBox::Yes:
                default:
                    break;
                }
            }
            c.range=fabs(rangeMax)>fabs(rangeMin)?fabs(rangeMax):fabs(rangeMin);
            proj->prototypes[protoNum].addConditions(c);
            //drawMembershipFunction(&c);
            myview->getScene()->clear();
            ui->lineEdit_lc->clear();
            ui->lineEdit_lu->clear();
            ui->lineEdit_hu->clear();
            ui->lineEdit_hc->clear();
            emit updatePrototype();
        }
    } else if(ui->radioButton_freehand->isChecked()){
        if(freeKnotX->size()>2){
            solim::Curve c = solim::Curve(ui->comboBox_cov->currentText().toStdString(),solim::CONTINUOUS,freeKnotX,freeKnotY);
            c.range=fabs(rangeMax)>fabs(rangeMin)?fabs(rangeMax):fabs(rangeMin);
            proj->prototypes[protoNum].addConditions(c);
            emit updatePrototype();
            freeKnotX->clear();
            freeKnotX->shrink_to_fit();
            freeKnotY->clear();
            freeKnotY->shrink_to_fit();
            myview->getScene()->clear();
            myview->editFreehandRule=false;
        }
    } else if(ui->radioButton_enum->isChecked()){
        if(enumVals.size()>0){
            solim::Curve c=solim::Curve(covname,solim::CATEGORICAL);
            for(int i = 0;i<enumVals.size();i++){
                c.addKnot(enumVals[i],1);
            }
            c.typicalValue=enumVals[0];
            c.range=fabs(rangeMax)>fabs(rangeMin)?fabs(rangeMax):fabs(rangeMin);
            proj->prototypes[protoNum].addConditions(c);
            myview->getScene()->clear();
            emit updatePrototype();
        }
    }
}

void AddPrototype_Expert::on_comboBox_curve_activated(const QString &arg1) {
    if(arg1=="Bell-shaped"){
        ui->lineEdit_lc->setEnabled(true);
        ui->lineEdit_lu->setEnabled(true);
        ui->lineEdit_hu->setEnabled(true);
        ui->lineEdit_hc->setEnabled(true);
    } else if(arg1=="S-shaped"){
        ui->lineEdit_lc->setEnabled(true);
        ui->lineEdit_lu->setEnabled(true);
        ui->lineEdit_hu->setEnabled(false);
        ui->lineEdit_hc->setEnabled(false);
    } else if(arg1=="Z-shaped"){
        ui->lineEdit_lc->setEnabled(false);
        ui->lineEdit_lu->setEnabled(false);
        ui->lineEdit_hu->setEnabled(true);
        ui->lineEdit_hc->setEnabled(true);
    }
}

void AddPrototype_Expert::drawMembershipFunction(solim::Curve *c) {
    ui->label_membership->setVisible(true);
    myview->setVisible(true);
    QGraphicsScene *scene = myview->getScene();
    scene->clear();
    scene->setSceneRect(0,0,myview->width()*0.9,myview->height()*0.9);
    QPen curvePen(Qt::blue);
    QPen axisPen(Qt::gray);
    axisPen.setWidth(2);
    curvePen.setWidth(1);
    int sceneWidth = scene->width();
    int sceneHeight = scene->height();
    myview->curveXMax = rangeMax;
    myview->curveXMin = rangeMin;

    // set axis
    scene->addLine(0.05*sceneWidth,0.85*sceneHeight,0.85*sceneWidth,0.85*sceneHeight,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight+1,0.85*sceneWidth-3,0.85*sceneHeight+4,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight,0.85*sceneWidth-3,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.1*sceneHeight,axisPen);
    scene->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth-3,0.1*sceneHeight+3,axisPen);
    scene->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth+3,0.1*sceneHeight+3,axisPen);
    // set label
    scene->addLine(0.80*sceneWidth,0.85*sceneHeight,0.80*sceneWidth,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.10*sceneWidth,0.15*sceneHeight,0.10*sceneWidth+3,0.15*sceneHeight,axisPen);

    // set axis names
    QGraphicsTextItem *yaxisName = scene->addText("Optimality value");
    yaxisName->setFont(QFont("Times", 10, QFont::Bold));
    yaxisName->setPos(0.10*sceneWidth, 0.1*sceneHeight-20);
    QGraphicsTextItem *xaxisName = scene->addText("Covariate value");
    xaxisName->setFont(QFont("Times", 10, QFont::Bold));
    xaxisName->setPos(0.85*sceneWidth, 0.85*sceneHeight-10);

    QGraphicsTextItem *yaxis1 = scene->addText("1");
    yaxis1->setFont(QFont("Times", 10, QFont::Bold));
    yaxis1->setPos(0.10*sceneWidth-15,0.15*sceneHeight-10);
    QGraphicsTextItem *yaxis0 = scene->addText("0");
    yaxis0->setFont(QFont("Times", 10, QFont::Bold));
    yaxis0->setPos(0.10*sceneWidth-20,0.85*sceneHeight-20);
    QGraphicsTextItem *xaxis1 = scene->addText(QString::number(rangeMax));
    xaxis1->setFont(QFont("Times", 10, QFont::Bold));
    xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
    QGraphicsTextItem *xaxis0 = scene->addText(QString::number(rangeMin));
    xaxis0->setFont(QFont("Times", 10, QFont::Bold));
    xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
    if(c==nullptr) return;
    double previousx,previousy;
    previousy = c->getOptimality(myview->curveXMin);
    previousx = myview->curveXMin;

    double x,y;
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;
    float range = myview->curveXMax-myview->curveXMin;
    for(int i =0;i<100;i++){
        x =i*range/101.0+myview->curveXMin;
        y = c->getOptimality(x);
        if(fabs(y+1)<VERY_SMALL ||fabs(previousy+1)<VERY_SMALL)
            continue;
        scene->addLine((previousx-myview->curveXMin)/range*graphWidth+xStart,yEnd-previousy*graphHeight,
                       (x-myview->curveXMin)/range*graphWidth+xStart,yEnd-y*graphHeight,curvePen);
        previousx = x;
        previousy = y;
    }
}

void AddPrototype_Expert::on_btn_add_opt_val_clicked() {
    if(ui->radioButton_enum->isChecked()){
        bool*toNumFlag = new bool;
        int num=ui->lineEdit_opt_val->text().toInt(toNumFlag);
        if(*toNumFlag){
            if(num>rangeMax || num<rangeMin){
                warn("Optimal value should be in the range of covariate value");
                return;
            }
            for(int i=0;i<enumVals.size();i++){
                if(num==enumVals[i])
                    return;
            }
            enumVals.push_back(num);
            myview->knotX.push_back(num);
            myview->knotY.push_back(1);
            onAddEnumPoint();
        }
    }
}

void AddPrototype_Expert::drawEnumRange(){
    ui->label_membership->setVisible(true);
    myview->setVisible(true);
    QGraphicsScene *scene = myview->getScene();
    scene->clear();
    scene->setSceneRect(0,0,myview->width()*0.9,myview->height()*0.9);
    QPen axisPen(Qt::gray);
    axisPen.setWidth(2);
    int sceneWidth = scene->width();
    int sceneHeight = scene->height();
    // set axis
    scene->addLine(0.05*sceneWidth,0.85*sceneHeight,0.85*sceneWidth,0.85*sceneHeight,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight+1,0.85*sceneWidth-3,0.85*sceneHeight+4,axisPen);
    scene->addLine(0.85*sceneWidth,0.85*sceneHeight,0.85*sceneWidth-3,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.10*sceneWidth,0.85*sceneHeight,0.10*sceneWidth,0.1*sceneHeight,axisPen);
    scene->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth-3,0.1*sceneHeight+3,axisPen);
    scene->addLine(0.10*sceneWidth,0.1*sceneHeight,0.10*sceneWidth+3,0.1*sceneHeight+3,axisPen);
    // set label
    scene->addLine(0.80*sceneWidth,0.85*sceneHeight,0.80*sceneWidth,0.85*sceneHeight-3,axisPen);
    scene->addLine(0.10*sceneWidth,0.15*sceneHeight,0.10*sceneWidth+3,0.15*sceneHeight,axisPen);
    // set axis names
    QGraphicsTextItem *yaxisName = scene->addText("Optimality value");
    yaxisName->setFont(QFont("Times", 10, QFont::Bold));
    yaxisName->setPos(0.10*sceneWidth, 0.1*sceneHeight-20);
    QGraphicsTextItem *xaxisName = scene->addText("Covariate value");
    xaxisName->setFont(QFont("Times", 10, QFont::Bold));
    xaxisName->setPos(0.85*sceneWidth, 0.85*sceneHeight-10);

    QGraphicsTextItem *yaxis1 = scene->addText("1");
    yaxis1->setFont(QFont("Times", 10, QFont::Bold));
    yaxis1->setPos(0.10*sceneWidth-15,0.15*sceneHeight-10);
    QGraphicsTextItem *yaxis0 = scene->addText("0");
    yaxis0->setFont(QFont("Times", 10, QFont::Bold));
    yaxis0->setPos(0.10*sceneWidth-20,0.85*sceneHeight-20);

    QGraphicsTextItem *xaxis1 = scene->addText(QString::number(int(rangeMax+1)));
    xaxis1->setFont(QFont("Times", 10, QFont::Bold));
    xaxis1->setPos(0.80*sceneWidth-4*xaxis1->toPlainText().size(),0.85*sceneHeight);
    QGraphicsTextItem *xaxis0 = scene->addText(QString::number(int(rangeMin)));
    xaxis0->setFont(QFont("Times", 10, QFont::Bold));
    xaxis0->setPos(0.10*sceneWidth-4*xaxis0->toPlainText().size(),0.85*sceneHeight);
    yaxisName->setPos(0.10*sceneWidth, 0.1*sceneHeight-20);
    myview->curveXMax = int(rangeMax+1);
    myview->curveXMin = int(rangeMin);
    /*if(ui->radioButton_enum->isChecked())
        ui->label_freehand_hint->setText("<b>Hint</b>: Enumerated rules are used for categorical covariables, e.g. geology, land use type."
                                         " Add all possible conditions when prototype occurs as optimality value");
    else if(ui->radioButton_freehand->isChecked())
        ui->label_freehand_hint->setText("<b>Hint</b>: Double click membership function view to add or delete points. Move points to adjust spline");
    else if(ui->radioButton_range->isChecked())
        ui->label_freehand_hint->setText("<b>Hint</b>: <i><b>Low cross/high cross</b></i>: The covariable values when optimality value decreased to 0.5.<br>"
                                         "<i><b>Low unity/high unity</b></i>: The covariable values when optimality value is 1.");*/
}

void AddPrototype_Expert::on_btn_reset_clicked()
{
    myview->knotX.clear();
    myview->knotY.clear();
    myview->knotX.shrink_to_fit();
    myview->knotY.shrink_to_fit();
    if(ui->radioButton_enum->isChecked()){
        enumVals.clear();
        enumVals.shrink_to_fit();
        myview->getScene()->clear();
        on_lineEdit_max_cov_textChanged(ui->lineEdit_max_cov->text());
    } else if(ui->radioButton_freehand->isChecked()){
        freeKnotX->clear();
        freeKnotX->shrink_to_fit();
        freeKnotY->clear();
        freeKnotY->shrink_to_fit();
        myview->getScene()->clear();
        on_lineEdit_max_cov_textChanged(ui->lineEdit_max_cov->text());
    } else if(ui->radioButton_range->isChecked()){
        solim::Curve c;
        bool warn;
        if(getPointRule(c,warn))
            drawMembershipFunction(&c);
    }
}

void AddPrototype_Expert::onAddFreehandPoint(){
    freeKnotX->clear();
    freeKnotX->shrink_to_fit();
    freeKnotY->clear();
    freeKnotY->shrink_to_fit();
    for(int i = 0; i<myview->knotX.size();i++){
        freeKnotX->push_back(myview->knotX[i]);
        freeKnotY->push_back(myview->knotY[i]);
    }
    int sceneWidth = myview->getScene()->width();
    int sceneHeight = myview->getScene()->height();
    int graphWidth = 0.7*sceneWidth;
    int graphHeight = 0.7*sceneHeight;
    int xStart = 0.10*sceneWidth;
    int yEnd = 0.85*sceneHeight;
    QPen pen(Qt::black);
    pen.setWidth(1);
    if(freeKnotX->size()>2){
        solim::Curve *c = new solim::Curve(ui->comboBox_cov->currentText().toStdString(),solim::CONTINUOUS,freeKnotX,freeKnotY);
        drawMembershipFunction(c);
        for(int i = 0; i<freeKnotX->size();i++){
            double x = (freeKnotX->at(i)-myview->curveXMin)/(myview->curveXMax-myview->curveXMin);
            double y = freeKnotY->at(i);
            myview->getScene()->addRect(x*graphWidth+xStart-2,yEnd-y*graphHeight-2,4,4,pen,QBrush(Qt::black));
        }
        delete c;
    }
    else {
        myview->getScene()->clear();
        drawMembershipFunction(nullptr);
        for(int i = 0; i<freeKnotX->size();i++){
            double x = (freeKnotX->at(i)-myview->curveXMin)/(myview->curveXMax-myview->curveXMin);
            double y = freeKnotY->at(i);
            myview->getScene()->addRect(x*graphWidth+xStart-2,yEnd-y*graphHeight-2,4,4,pen,QBrush(Qt::black));
        }
    }
}

void AddPrototype_Expert::onAddEnumPoint(){
    myview->getScene()->clear();
    drawEnumRange();
    int margin = fabs(rangeMax)>fabs(rangeMin)?fabs(rangeMax):fabs(rangeMin);
    QGraphicsScene *scene = myview->getScene();
    int xStart = 0.10*scene->width();
    int graphWidth = 0.7*scene->width();
    QPen curvePen(Qt::blue);
    curvePen.setWidth(2);
    enumVals.clear();
    enumVals.shrink_to_fit();
    if(rangeMax*rangeMin<0||!rangeMax>0){
        for(int i = 0; i<myview->knotX.size();i++){
            int num = myview->knotX[i];
            if(num>rangeMax) num=rangeMax;
            if(num<rangeMin) num=rangeMin;
            for(int i = 0;i<enumVals.size();i++){
                if(num==enumVals[i]) return;
            }
            enumVals.push_back(num);
            scene->addLine(0.5*(num+margin)/margin*graphWidth+xStart,0.85*scene->height(),0.5*(num+margin)/margin*graphWidth+xStart,0.15*scene->height(),curvePen);
            QGraphicsTextItem *tag = scene->addText(QString::number(num));
            tag->setFont(QFont("Times", 8));
            tag->setDefaultTextColor(Qt::blue);
            tag->setPos(0.5*(num+margin)/margin*graphWidth+xStart-4*tag->toPlainText().size(),0.85*scene->height());
        }
    }else{
        for(int i = 0; i<myview->knotX.size();i++){
            int num = myview->knotX[i];
            for(int i = 0;i<enumVals.size();i++){
                if(num==enumVals[i]) return;
            }
            enumVals.push_back(num);
            scene->addLine(1.0*num/rangeMax*graphWidth+xStart,0.85*scene->height(),1.0*num/rangeMax*graphWidth+xStart,0.15*scene->height(),curvePen);
            QGraphicsTextItem *tag = scene->addText(QString::number(num));
            tag->setFont(QFont("Times", 8));
            tag->setDefaultTextColor(Qt::blue);
            tag->setPos(1.0*num/rangeMax*graphWidth+xStart-4*tag->toPlainText().size(),0.85*scene->height());
        }
    }
}

void AddPrototype_Expert::on_lineEdit_min_cov_textChanged(const QString &arg1)
{
    if((!ui->lineEdit_max_cov->text().isEmpty())&&(!ui->lineEdit_min_cov->text().isEmpty())){
        if(ui->comboBox_datatype->currentIndex()==1){
            ui->radioButton_enum->setEnabled(true);
            ui->radioButton_enum->setChecked(true);
        } else {
            ui->radioButton_range->setEnabled(true);
            ui->radioButton_freehand->setEnabled(true);
        }
    } else {
        ui->radioButton_enum->setEnabled(false);
        ui->radioButton_freehand->setEnabled(false);
        ui->radioButton_range->setEnabled(false);
    }
    bool *toNum = new bool;
    double min = arg1.toDouble(toNum);
    if(!*toNum) return;
    double max = ui->lineEdit_max_cov->text().toDouble(toNum);
    if(!*toNum) return;
    if(max<min||fabs(max-min)<0.0001) return;
    rangeMax=max;
    rangeMin=min;
    if(ui->radioButton_freehand->isChecked()||ui->radioButton_enum->isChecked()){
        drawEnumRange();
    }
}

void AddPrototype_Expert::on_lineEdit_max_cov_textChanged(const QString &arg1)
{
    if((!ui->lineEdit_max_cov->text().isEmpty())&&(!ui->lineEdit_min_cov->text().isEmpty())){
        if(ui->comboBox_datatype->currentIndex()==1){
            ui->radioButton_enum->setEnabled(true);
            ui->radioButton_enum->setChecked(true);
        } else {
            ui->radioButton_range->setEnabled(true);
            ui->radioButton_freehand->setEnabled(true);
        }
    } else {
        ui->radioButton_enum->setEnabled(false);
        ui->radioButton_freehand->setEnabled(false);
        ui->radioButton_range->setEnabled(false);
    }
    bool *toNum = new bool;
    double max = arg1.toDouble(toNum);
    if(!*toNum) return;
    double min = ui->lineEdit_min_cov->text().toDouble(toNum);
    if(!*toNum) return;
    if(max<min||fabs(max-min)<0.0001) return;
    rangeMax=max;
    rangeMin=min;
    if(ui->radioButton_enum->isChecked()){
        drawEnumRange();
    }
    if(ui->radioButton_freehand->isChecked()){
        drawMembershipFunction(nullptr);
    }
}

bool AddPrototype_Expert::getPointRule(solim::Curve &c, bool &warn){
    warn = false;
    if(!ui->radioButton_range->isChecked()) return false;
    solim::CurveTypeEnum type=solim::BELL_SHAPED;
    float lu=-1,lc=-1,hu=-1,hc=-1;
    bool*toNumFlag = new bool;
    int checkflag = true;
    float max,min;
    max = ui->lineEdit_max_cov->text().toDouble(toNumFlag);
    if(!*toNumFlag) checkflag = false;
    if(checkflag) {
        min = ui->lineEdit_min_cov->text().toDouble(toNumFlag);
        if(!*toNumFlag) checkflag = false;
    }
    if(ui->comboBox_curve->currentText()=="Bell-shaped"){
        lu=ui->lineEdit_lu->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        lc=ui->lineEdit_lc->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        hu=ui->lineEdit_hu->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        hc=ui->lineEdit_hc->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        if(!(hc>hu)) return false;
        if(!(lu>lc)) return false;
        if(hu<lu) return false;
        type = solim::BELL_SHAPED;
        if(checkflag){
            if(hc<min || lc>max)
                warn = true;
        }
    } else if(ui->comboBox_curve->currentText()=="S-shaped"){
        lu=ui->lineEdit_lu->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        lc=ui->lineEdit_lc->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        if(!(lu>lc)) return false;
        hu=-1;
        hc=-1;
        type=solim::S_SHAPED;
        if(checkflag){
            if(lu<min || lc>max)
                warn = true;
        }
    } else if(ui->comboBox_curve->currentText()=="Z-shaped"){
        bool*toNumFlag = new bool;
        hu=ui->lineEdit_hu->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        hc=ui->lineEdit_hc->text().toDouble(toNumFlag);
        if(!*toNumFlag) return false;
        if(!(hc>hu)) return false;
        lu=-1;
        lc=-1;
        type=solim::Z_SHAPED;
        if(checkflag){
            if(hc<min || hu>max)
                warn = true;
        }
    }
    c=solim::Curve(ui->comboBox_cov->currentText().toStdString(),lu,hu,lc,hc,type);
    return true;
}

void AddPrototype_Expert::on_comboBox_datatype_activated(int index)
{
    if((!ui->lineEdit_max_cov->text().isEmpty())&&(!ui->lineEdit_min_cov->text().isEmpty())){
        if(index==1){
            ui->radioButton_enum->setEnabled(true);
            ui->radioButton_range->setEnabled(false);
            ui->radioButton_freehand->setEnabled(false);
            ui->radioButton_enum->setChecked(true);
        } else {
            ui->radioButton_enum->setEnabled(false);
            ui->radioButton_range->setEnabled(true);
            ui->radioButton_freehand->setEnabled(true);
            myview->getScene()->clear();
            if(myview->isVisible()){
                ui->radioButton_freehand->setChecked(true);
            }
        }
    }
}

void AddPrototype_Expert::on_comboBox_datatype_currentIndexChanged(int index)
{
    on_comboBox_datatype_activated(index);
}

void AddPrototype_Expert::on_btn_create_clicked()
{
    if(!ui->lineEdit_prototype->text().isEmpty()){
        solim::Prototype prop;
        prop.prototypeID=ui->lineEdit_prototype->text().toStdString();
        prop.source=solim::EXPERT;
        prop.prototypeBaseName=currentBasename;
        proj->prototypes.push_back(prop);
        protoNum=proj->prototypes.size()-1;
        ui->btn_add_prop->setEnabled(true);
        ui->comboBox_cov->setEnabled(true);
        ui->btn_create->setEnabled(false);
        ui->lineEdit_prototype->setReadOnly(true);
        emit updatePrototype();
        QString suggestedPropName = "";
        solim::DataTypeEnum suggestedPropType = solim::CONTINUOUS;
        for(int i = 0; i< proj->prototypes.size();i++){
            if(proj->prototypes[i].prototypeBaseName==currentBasename){
                if(proj->prototypes[i].properties.size()>0){
                    suggestedPropName = proj->prototypes[i].properties[0].propertyName.c_str();
                    suggestedPropType = proj->prototypes[i].properties[0].soilPropertyType;
                    break;
                }
            }
        }
        if(suggestedPropName.isEmpty()){
            for(int i = 0; i< proj->prototypes.size();i++){
                if(proj->prototypes[i].prototypeBaseName!=currentBasename){
                    if(proj->prototypes[i].properties.size()>0){
                        suggestedPropName = proj->prototypes[i].properties[0].propertyName.c_str();
                        suggestedPropType = proj->prototypes[i].properties[0].soilPropertyType;
                        break;
                    }
                }
            }
        }
        ui->lineEdit_prop_name->setText(suggestedPropName);
        if(suggestedPropType==solim::CATEGORICAL)
            ui->checkBox_datatype_category->setCheckState(Qt::Checked);
        else
            ui->checkBox_datatype_category->setCheckState(Qt::Unchecked);
    }
}


void AddPrototype_Expert::on_lineEdit_lc_textChanged(const QString &arg1)
{
    solim::Curve c;
    bool warn;
    if(getPointRule(c,warn))
        drawMembershipFunction(&c);
}

void AddPrototype_Expert::on_lineEdit_lu_textChanged(const QString &arg1)
{
    solim::Curve c;
    bool warn;
    if(getPointRule(c,warn))
        drawMembershipFunction(&c);
}

void AddPrototype_Expert::on_lineEdit_hu_textChanged(const QString &arg1)
{
    solim::Curve c;
    bool warn;
    if(getPointRule(c,warn))
        drawMembershipFunction(&c);
}

void AddPrototype_Expert::on_lineEdit_hc_textChanged(const QString &arg1)
{
    solim::Curve c;
    bool warn;
    if(getPointRule(c,warn))
        drawMembershipFunction(&c);
}

void AddPrototype_Expert::on_comboBox_curve_activated(int index)
{
    solim::Curve c;
    bool warn;
    if(getPointRule(c,warn))
        drawMembershipFunction(&c);
}
