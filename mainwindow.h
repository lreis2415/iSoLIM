#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QDockWidget>
#include <QTreeView>
#include <QScrollBar>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QTableView>
#include <QHeaderView>
#include <QToolBar>
#include <QTextCodec>
#include "mapinference.h"
#include "addprototypebase.h"
#include "addprototype_expert.h"
#include "mygraphicsview.h"
#include "newprojectdialog.h"
#include "simpledialog.h"
#include "changecovname.h"
#include "validation.h"
#include "legendwindow.h"
#include <iostream>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStandardItemModel* model;
    QMenu* prototypesMenu;
    QMenu* gisDataMenu;
    QMenu* gisLayerMenu;
    QMenu* prototypeBaseMenu;
    QMenu* prototypeMenu;
    QMenu* membershipMenu;
    QStandardItem *resultChild;
    QStandardItem *prototypeChild;
    QStandardItem *gisDataChild;
    SoLIMProject *proj;
    QString workingDir;
    QThread createImgThread;
private slots:
    // project tree slots
    void onSelectionChanged(const QItemSelection&,const QItemSelection&);
    void onCustomContextMenu(const QPoint &point);
    void onImportPrototypeBase();
    void onAddGisData();
    void onRenamePrototypeBase();
    void onChangeCovName(); // change for prototype
    void onSavePrototypeBase();
    void onExportPrototypeBase();
    void onAddRules();
    void onDeletePrototypeBase();
    void onDeletePrototype();
    void onDeleteGisLayer();
    void onModifyCovName();
    void onModifyCovFile();
    void onEditRule();
    void onDeleteRule();
    void saveEditRuleChange();
    void onAddFreehandPoint();
    void resetEditRule();
    void onAddEnumPoint();
    //prototype from expert
    void onCreatePrototypeFromExpert();
    void onGetGisData();
    void onGetPrototype();
    void onUpdatePrototypeFromExpert(const Prototype *prop);
    //graphics view
    void onZoomin();
    void onZoomout();
    void onLookup();
    void onShowLegend(bool disableLegend = false);
    void onInferResults();
    void createImg();
    void finishedCreateImg();
    void onResetRange();
    void onExpanded(const QModelIndex&);

    void on_actionAdd_Covariates_triggered();
    void on_actionResample_triggered();
    void on_actionValidation_triggered();
    void on_action_infer_triggered();
    void on_actionView_Data_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_as_triggered();
    void on_actionDefine_Study_Area_triggered();
    void on_actionAdd_prototypes_from_samples_triggered();
    void on_actionAdd_prototypes_from_expert_triggered();
    void on_actionAdd_prototypes_from_Data_Mining_triggered();

    void on_actionClose_Project_triggered();

private:
    Ui::MainWindow *ui;
    LegendWindow *legendView;
    QTreeView *projectView;
    QDockWidget *projectDock;
    QDockWidget *dataDetailsDock;
    QTableView *dataDetailsView;
    bool projectViewInitialized;
    bool projectSaved;
    MyGraphicsView *myGraphicsView;
    QImage *img;    // store pointer for current showing image
    string imgFilename; // store current showing image filename
    QToolBar *zoomToolBar;
    QToolBar *resetRangeToolBar;
    QAction *addProtoExpert;
    QAction *addRules;
    QAction *editRule;
    QAction *saveRule;
    QAction *resetRule;
    string currentBaseName;
    string currentProtoName;
    string currentLayerName;
    string gisDataName;
    AddPrototype_Expert *addRule;
    BaseIO *lyr;
    double imgMax;
    double imgMin;
    string graphicFilename;
    void initialProjectView();
    void initDataDetailsView();
    void initModel();
    bool saveWarning();
    bool baseExistsWarning(string basename);
    void readPrototype(TiXmlElement*prototypesElement);
    bool drawLayer(string filename);
    void drawMembershipFunction(float max = NODATA, float min = NODATA, solim::Curve *c = nullptr);
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }
    void initParas();
    void saveSetting();
    void clearView();
};
#endif // MAINWINDOW_H
