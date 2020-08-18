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
#include "inference.h"
#include "project.h"
#include "prototypefromsamples.h"
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
    QMenu* viewDataMenu;
    QAction* viewData;
    QStandardItem *resultChild;
    QStandardItem *prototypeChild;
    SoLIMProject *proj;
private slots:
    void onSoilInferenceFromSample();
    void onProjectNew();
    void onProjectSave();
    void onProjectOpen();
    void onAddPrototypeFromSamples();
    void onCustomContextMenu(const QPoint &point);
    void onGetPrototype();
    void onInferResults();
    void onViewData();
    void onSelectionChanged(const QItemSelection&,const QItemSelection&);

private:
    Ui::MainWindow *ui;
    QTreeView *projectView;
    QDockWidget *projectDock;
    //soilInference *inference;
    prototypeFromSamples *getPrototype;
    string projectFileName;
    void drawLayer(string filename);
    void drawMembershipFunction(string basename, string idname, string covName);
    void initialProjectView();
    bool saveWarning();
    bool projectViewInitialized;
    bool projectSaved;
};
#endif // MAINWINDOW_H
