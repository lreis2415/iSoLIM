#include "legendwindow.h"
#include "ui_legendwindow.h"

LegendWindow::LegendWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::legendwindow)
{
    ui->setupUi(this);
    legendView = new QGraphicsView(this);
}

LegendWindow::~LegendWindow()
{
    delete ui;
}
void LegendWindow::updateview(double max, double min){
    QGraphicsScene *scene = new QGraphicsScene();
    scene->setSceneRect(0,0,70,80);
    legendView->setScene(scene);
    QLinearGradient linear(QPoint(5,15),QPoint(5,65));
    linear.setColorAt(0,Qt::white);
    linear.setColorAt(1,Qt::black);
    linear.setSpread(QGradient::PadSpread);
    scene->addRect(5,15,10,50,QPen(QColor(255,255,255),0),linear);
    //std::ostringstream maxss;
    int precision = 0;
    if(max-min<10) precision = 2;
    QGraphicsTextItem *minLabel = scene->addText(QString::number(min,'f',precision));
    minLabel->setPos(15,50);
    QGraphicsTextItem *maxLabel = scene->addText(QString::number(max,'f',precision));
    maxLabel->setPos(15,0);
}
