#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "soilinferencefromsamples.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onSoilInferenceFromSample();

private:
    Ui::MainWindow *ui;
    soilInferenceFromSamples *inferFromSamples;
};
#endif // MAINWINDOW_H
