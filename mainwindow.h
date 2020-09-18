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
#include "inference.h"
#include "project.h"
#include "prototypefromsamples.h"
#include "mygraphicsview.h"
#include "newprojectdialog.h"
#include "addgisdatadialog.h"
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
    QMenu* prototypeMenu;
    QAction *prototypesFromSamples;
    QAction *addPrototype;
    QAction *addExclusion;
    QAction *addOccurrence;
    QMenu* gisDataMenu;
    QAction* addGisData;
    QStandardItem *resultChild;
    QStandardItem *prototypeChild;
    QStandardItem *gisDataChild;
    SoLIMProject *proj;
private slots:
    void onSoilInferenceFromPrototypes();
    void onProjectNew();
    void onProjectSave();
    void onProjectOpen();
    void onAddPrototypeFromSamples();
    void onCustomContextMenu(const QPoint &point);
    void onGetPrototype();
    void onInferResults();
    void onViewData();
    void onSelectionChanged(const QItemSelection&,const QItemSelection&);
    void onZoomin();
    void onZoomout();
    void onAddGisData();
    void onGetGisData();

    //void on_layerInfo_btn_clicked();

private:
    Ui::MainWindow *ui;
    QTreeView *projectView;
    QDockWidget *projectDock;
    QDockWidget *dataDetailsDock;
    QTableView *dataDetailsView;
    prototypeFromSamples *getPrototype;
    MyGraphicsView *myGraphicsView;
    string projectFileName;
    QImage *img;
    bool projectViewInitialized;
    bool projectSaved;
    string imgFilename;
    double imgMax,imgMin;
    QToolBar *zoomToolBar;
    void drawLayer(string filename);
    void drawMembershipFunction(string basename, string idname, string covName);
    void initialProjectView();
    bool saveWarning();
    void initDataDetailsView();
    void initModel();
};
#endif // MAINWINDOW_H
