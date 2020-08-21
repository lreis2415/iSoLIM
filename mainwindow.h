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
#include <QMouseEvent>
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
    void mouseMoveEvent(QMouseEvent *event);

    void on_zoomin_btn_clicked();

    void on_zoomout_btn_clicked();

    void on_layerInfo_btn_clicked();

private:
    Ui::MainWindow *ui;
    QTreeView *projectView;
    QDockWidget *projectDock;
    prototypeFromSamples *getPrototype;
    string projectFileName;
    QImage *img;
    void drawLayer(string filename);
    void drawMembershipFunction(string basename, string idname, string covName);
    void initialProjectView();
    bool saveWarning();
    bool projectViewInitialized;
    bool projectSaved;
    bool clickForInfo;
};
#endif // MAINWINDOW_H
