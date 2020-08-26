#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTableView>
#include <string>
#include <QStandardItemModel>
#include <QStandardItem>
#include <solim-lib-forqt.h>
class MyGraphicsView:public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget *parent = 0);
    bool clickForInfo;
    string filename;
    BaseIO *lyr;
    QImage *img;
    double imgMax;
    double imgMin;
    double range;
    QTableView *dataDetailsView;
    QStandardItemModel *dataDetailsModel;
    QGraphicsScene *getScene(){ return scene; }
signals:
public slots:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    QGraphicsScene *scene;
};

#endif // MYGRAPHICSVIEW_H
