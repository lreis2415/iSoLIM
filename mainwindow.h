#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "soilinferencefromsamples.h"
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
    QAction *addPrototype;
    QAction *addExclusion;
    QAction *addOccurrence;
    QStandardItem *resultChild;
    QStandardItem *prototypeChild;
    SoLIMProject *proj;
private slots:
    void onSoilInferenceFromSample();
    void onProjectNew();
    void onProjectSave();
    void onProjectOpen();
    void onAddPrototype();
    void onCustomContextMenu(const QPoint &point);
    void onGetPrototype();

private:
    Ui::MainWindow *ui;
    soilInferenceFromSamples *inferFromSamples;
    prototypeFromSamples *getPrototype;
    string projectFileName;
    void drawLayer(string filename);
};
#endif // MAINWINDOW_H
