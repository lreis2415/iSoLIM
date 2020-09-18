#include "mygraphicsview.h"
#include <QDebug>

MyGraphicsView::MyGraphicsView(QWidget *parent):
    QGraphicsView(parent)
{
    scene = new QGraphicsScene;
    this->setScene(scene);
    clickForInfo = false;
    filename = "";
    lyr = nullptr;
    img = nullptr;
    dataDetailsView = nullptr;
    imgMax = 0;
    imgMin = 0;
    showImage = false;
}
void MyGraphicsView::mousePressEvent(QMouseEvent * e)
{

    if(clickForInfo){
        QPointF pt = mapToScene(e->pos());

        int strechWidth = img->scaled(scene->width()-30,scene->height(),Qt::KeepAspectRatio).width();
        int strechHeight = img->scaled(scene->width()-30,scene->height(),Qt::KeepAspectRatio).height();
        int row = pt.y()/strechHeight*img->height();
        int col = (pt.x()-30)/strechWidth*(img->width());
        if(row<img->height()&&col<img->width()){
            float value = lyr->getValue(col,row);
            if(value<NODATA||fabs(value-NODATA)<VERY_SMALL){
                return;
            }
            double geoX,geoY;
            lyr->globalXYToGeo(col,row,geoX,geoY);
            QMessageBox msg;
            QString msg_str = "Location:\n    column: ";
            msg_str.append(QString::number(col));
            msg_str.append("; row: ");
            msg_str.append(QString::number(row));
            msg_str.append("\n    x: ");
            msg_str.append(QString::number(geoX));
            msg_str.append("; y: ");
            msg_str.append(QString::number(geoY));
            msg_str.append("\n");
            msg_str.append("Value: ");
            msg_str.append(to_string(value).c_str());
            msg.setText(msg_str);
            msg.exec();
            clickForInfo = false;
        }
    }
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent * e){
    if(!showImage)
        return;
    if(img&&dataDetailsView){
        QPointF pt = mapToScene(e->pos());
        QImage tempimg = img->scaled(scene->width()-30,scene->height(),Qt::KeepAspectRatio);
        int strechWidth = tempimg.width();
        int strechHeight = tempimg.height();
        int row = pt.y()/strechHeight*tempimg.height();
        int col = (pt.x()-30)/strechWidth*(tempimg.width());
        if(row<tempimg.height()-1&&col<tempimg.width()-1&&row>0&&col>0){
            QStandardItemModel*model = new QStandardItemModel(dataDetailsView);

            for(int i = -4; i<5;i++){
                for(int j = -4;j<5;j++){
                    int pixel = tempimg.pixelColor(col+i,row+j).red();
                    if(pixel == 255){
                                    //dataDetailsModel->setItem(i,j, new QStandardItem(""));
                        model->setItem(i+4,j+4, new QStandardItem("NoData"));
                    } else {
                        if(range>100)
                            model->setItem(i+4,j+4,new QStandardItem(QString::number(int(pixel*range+imgMin))));
                        else
                            model->setItem(i+4,j+4,new QStandardItem(QString::number(pixel*range+imgMin,'g',4)));
                        //((QStandardItemModel*)dataDetailsView->model())->item(i+1,j+1)->setData(QString::number(int(pixel*range+imgMin)));
                    }
                    if(i==0&&j==0)
                        model->item(i+4,j+4)->setForeground(QBrush(QColor(255, 0, 0)));
                }
            }
            dataDetailsView->setModel(model);
            dataDetailsView->resizeColumnsToContents();
        }
    }
}
