#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QDockWidget>
#include <QTreeView>
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
#include "mapinference.h"
#include "addprototypebase.h"
#include "addrule.h"
#include "mygraphicsview.h"
#include "newprojectdialog.h"
#include "simpledialog.h"
#include "changecovname.h"
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
    QStandardItem *resultChild;
    QStandardItem *prototypeChild;
    QStandardItem *gisDataChild;
    SoLIMProject *proj;
    QString workingDir;
    QThread createImgThread;
private slots:
    // main menu
    void onProjectNew();
    void onProjectSave();
    void onProjectOpen();
    void onProjectSaveAs();
    void onViewData();
    void onSoilInferenceFromPrototypes();
    void onEditStudyArea();
    // project tree slots
    void onSelectionChanged(const QItemSelection&,const QItemSelection&);
    void onCustomContextMenu(const QPoint &point);
    void onAddPrototypeBaseFromSamples();
    void onAddPrototypeBaseFromExpert();
    void onAddPrototypeBaseFromMining();
    void onImportPrototypeBase();
    void onAddGisData();
    void onChangeCovName();
    void onSavePrototypeBase();
    void onExportPrototypeBase();
    void onAddRules();
    void onDeletePrototypeBase();
    void onDeletePrototype();
    //prototype from expert
    void onCreatePrototypeFromExpert();
    void onGetGisData();
    void onGetPrototype();
    void onUpdatePrototypeFromExpert(const Prototype *prop);
    //graphics view
    void onZoomin();
    void onZoomout();
    void onInferResults();
    void on_actionAdd_Covariates_triggered();
    void createImg();
    void finishedCreateImg();
    void onDeleteGisLayer();
    void onModifyCovFile();

    void on_actionResample_triggered();

private:
    Ui::MainWindow *ui;
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
    string currentBaseName;
    string currentProtoName;
    string currentLayerName;
    AddRule *addRule;
    BaseIO *lyr;
    double imgMax;
    double imgMin;
    string graphicFilename;
    void initialProjectView();
    void initDataDetailsView();
    void initModel();
    bool saveWarning();
    bool baseExistsWarning(string basename);
    void updateGisDataFromTree();
    void readPrototype(TiXmlElement*prototypesElement);
    void drawLayer(string filename);
    void drawMembershipFunction(string basename, string idname, string covName);
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }
    void initParas();
    void saveSetting();
};
#endif // MAINWINDOW_H
