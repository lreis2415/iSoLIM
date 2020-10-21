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
    string filename;
    BaseIO *lyr;
    QImage *img;
    double imgMax;
    double imgMin;
    double range;
    bool showImage;
    bool editFreehandRule;
    bool editEnumRule;
    QTableView *dataDetailsView;
    QStandardItemModel *dataDetailsModel;
    QGraphicsScene *getScene(){ return scene; }
    vector<double> knotX;
    vector<double> knotY;
    int moveKnotNum;
    int enumMax;
    int enumMin;
signals:
    void addFreehandPoint();
    void addEnumPoint();
public slots:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    QGraphicsScene *scene;
};

#endif // MYGRAPHICSVIEW_H
