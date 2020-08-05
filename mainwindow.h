#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "soilinferencefromsamples.h"
#include "project.h"
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
    QMenu* gisDataMenu;
    QAction *addLayer;
    QMenu* prototypeMenu;
    QAction *addPrototype;
    QAction *addExclusion;
    QAction *addOccurrence;
    QStandardItem *gisDataChild;
    QStandardItem *prototypeChild;
    SoLIMProject proj;
private slots:
    void onSoilInferenceFromSample();
    void onProjectNew();
    void onProjectSave();
    void onProjectOpen();
    void onAddLayer();
    void onAddPrototype();
    void onAddExclusion();
    void onAddOccurrence();
    void onCustomContextMenu(const QPoint &point);

private:
    Ui::MainWindow *ui;
    soilInferenceFromSamples *inferFromSamples;
    string projectFileName;
    void drawLayer(string filename);
};
#endif // MAINWINDOW_H
