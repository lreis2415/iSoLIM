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
    QMenu* prototypeMenu;
    QMenu* gisDataMenu;
    QMenu* prototypeBaseMenu;
    QStandardItem *resultChild;
    QStandardItem *prototypeChild;
    QStandardItem *gisDataChild;
    SoLIMProject *proj;
private slots:
    // main menu
    void onProjectNew();
    void onProjectSave();
    void onProjectOpen();
    void onProjectSaveAs();
    void onViewData();
    void onSoilInferenceFromPrototypes();
    // project tree slots
    void onSelectionChanged(const QItemSelection&,const QItemSelection&);
    void onCustomContextMenu(const QPoint &point);
    void onAddPrototypeFromSamples();
    void onImportPrototypeBase();
    void onAddGisData();
    void onChangeCovName();
    void onSavePrototypeBase();
    void onExportPrototypeBase();
    //graphics view
    void onZoomin();
    void onZoomout();

private:
    Ui::MainWindow *ui;
    QTreeView *projectView;
    QDockWidget *projectDock;
    QDockWidget *dataDetailsDock;
    QTableView *dataDetailsView;
    bool projectViewInitialized;
    bool projectSaved;
    prototypeFromSamples *getPrototype; // pointer to get prototype window
    MyGraphicsView *myGraphicsView;
    QImage *img;    // store pointer for current showing image
    string imgFilename; // store current showing image filename
    QToolBar *zoomToolBar;
    string currentBaseName;
    void initialProjectView();
    void initDataDetailsView();
    void initModel();
    bool saveWarning();
    void wrongFormatWarning();
    void updateGisDataFromTree();
    void onGetGisData();
    void onInferResults();
    void onGetPrototype();
    void drawLayer(string filename);
    void drawMembershipFunction(string basename, string idname, string covName);
};
#endif // MAINWINDOW_H
