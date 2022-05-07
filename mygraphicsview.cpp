#include "mygraphicsview.h"
#include <QDebug>

MyGraphicsView::MyGraphicsView(QWidget *parent):
    QGraphicsView(parent)
{
    scene = new QGraphicsScene;
    this->setScene(scene);
    setSceneRect(scene->sceneRect());
    filename = "";
    lyr = nullptr;
    img = nullptr;
    dataDetailsView = nullptr;
    membership=new solim::Curve();
    imgMax = 0;
    imgMin = 0;
    showImage = false;
    editFreehandRule=false;
    editEnumRule=false;
    knotX.clear();
    knotX.shrink_to_fit();
    knotY.clear();
    knotY.shrink_to_fit();
    offsetSize = 3;
}
void MyGraphicsView::mousePressEvent(QMouseEvent * e)
{
    if(editFreehandRule){
        moveKnotNum=-1;
        QPointF pt = mapToScene(e->pos());
        int sceneWidth = scene->width();
        int sceneHeight = scene->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        if(pt.x()<xStart||pt.x()>xStart+graphWidth) return;
        double x = 1.0*(pt.x()-xStart)/graphWidth;
        x = x * (curveXMax-curveXMin)+curveXMin;
        double y = 1.0*(yEnd-pt.y())/graphHeight;
        int py=pt.y();
        if(y>1){
            y=1;
            py=yEnd;
        }
        if(y<0){
            y=0;
            py=yEnd-sceneHeight;
        }
        for(int i = 0;i<knotX.size();i++){
            int knotXPt = (knotX[i]-curveXMin)/(curveXMax-curveXMin)*graphWidth+xStart;
            int knotYPt = yEnd - knotY[i]*graphHeight;
            if(fabs(knotXPt-pt.x())<offsetSize){
                if(fabs(knotYPt-pt.y())<offsetSize){
                    moveKnotNum=i;
                    return;
                }
            }
        }
    }
    else if(editEnumRule){
        moveKnotNum=-1;
        QPointF pt = mapToScene(e->pos());
        int sceneWidth = scene->width();
        int sceneHeight = scene->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        if(pt.x()<xStart||pt.x()>xStart+graphWidth) return;
        double x = 1.0*(pt.x()-xStart)/graphWidth;
        double y = 1.0*(yEnd-pt.y())/graphHeight;
        int py=pt.y();
        if(y>1||y<0) return;
        x = x * (curveXMax-curveXMin) + curveXMin;
        for(int i = 0;i<knotX.size();i++){
            int knotXPt = (knotX[i]-curveXMin)/(curveXMax-curveXMin)*graphWidth+xStart;
            if(fabs(knotXPt-pt.x())<offsetSize){
                moveKnotNum=i;
                return;
            }
        }
    }
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent * e){
    if(showImage){
        if(img&&dataDetailsView){
            QPointF pt = mapToScene(e->pos());
            QImage tempimg = img->scaled(scene->width()-30,scene->height(),Qt::KeepAspectRatio);
            int strechWidth = tempimg.width();
            int strechHeight = tempimg.height();
            int row = pt.y()/strechHeight*tempimg.height();
            int col = (pt.x()-30)/strechWidth*(tempimg.width());
            if(row<tempimg.height()-1&&col<tempimg.width()-1&&row>0&&col>0){
                QStandardItemModel*model = new QStandardItemModel(dataDetailsView);
                int neighbor=3;
                for(int i = 0-neighbor; i<=neighbor;i++){
                    for(int j = 0-neighbor;j<=neighbor;j++){
                        int pixel = tempimg.pixelColor(col+i,row+j).red();
                        if(pixel == 255){
                                        //dataDetailsModel->setItem(i,j, new QStandardItem(""));
                            model->setItem(i+neighbor,j+neighbor, new QStandardItem("NoData"));
                        } else {
                            if(imgMax>10)
                                model->setItem(i+neighbor,j+neighbor,new QStandardItem(QString::number(int(pixel*range+imgMin))));
                            else
                                model->setItem(i+neighbor,j+neighbor,new QStandardItem(QString::number(pixel*range+imgMin,'g',4)));
                            //((QStandardItemModel*)dataDetailsView->model())->item(i+1,j+1)->setData(QString::number(int(pixel*range+imgMin)));
                        }
                        if(i==0&&j==0)
                            model->item(i+neighbor,j+neighbor)->setForeground(QBrush(QColor(255, 0, 0)));
                    }
                }
                dataDetailsView->setModel(model);
                dataDetailsView->resizeColumnsToContents();
            }
        }
    }
    if(editFreehandRule&&moveKnotNum>-1){
        QPointF pt = mapToScene(e->pos());
        int sceneWidth = scene->width();
        int sceneHeight = scene->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        if(pt.x()<xStart||pt.x()>xStart+graphWidth) return;
        double x = 1.0*(pt.x()-xStart)/graphWidth;
        x = x * (curveXMax-curveXMin)+curveXMin;
        double y = 1.0*(yEnd-pt.y())/graphHeight;
        int py=pt.y();
        if(y>1){
            y=1;
            py=yEnd;
        }
        if(y<0){
            y=0;
            py=yEnd-sceneHeight;
        }
        for(int i = 0;i<knotX.size();i++){
            if(i!=moveKnotNum){
                int knotXPt = (knotX[i]-curveXMin)/(curveXMax-curveXMin)*graphWidth+xStart;
                if(fabs(knotXPt-pt.x())<offsetSize){
                    return;
                }
            }
        }
        knotX[moveKnotNum]=x;
        knotY[moveKnotNum]=y;
        emit addFreehandPoint();
    }
    else if(editEnumRule&&moveKnotNum>-1){
        QPointF pt = mapToScene(e->pos());
        int sceneWidth = scene->width();
        int sceneHeight = scene->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        if(pt.x()<xStart||pt.x()>xStart+graphWidth) return;
        double x = 1.0*(pt.x()-xStart)/graphWidth;
        double y = 1.0*(yEnd-pt.y())/graphHeight;
        int py=pt.y();
        if(y>1||y<0) return;
        x = x * (curveXMax-curveXMin) + curveXMin;
        for(int i = 0;i<knotX.size();i++){
            if(i!=moveKnotNum){
                int knotXPt = (knotX[i]-curveXMin)/(curveXMax-curveXMin)*graphWidth+xStart;
                if(fabs(knotXPt-pt.x())<offsetSize){
                    knotX.erase(knotX.begin()+moveKnotNum);
                    knotX.shrink_to_fit();
                    emit addEnumPoint();
                    return;
                }
            }
        }
        for(int i = 0;i<knotX.size();i++){
            if(knotX[i]==int(x+0.5))
                return;
        }
        knotX[moveKnotNum]=int(x+0.5);
        emit addEnumPoint();
    }
    else if(showMembership){
        if(membership!=nullptr&&dataDetailsView){
            int xpos=mapToScene(e->pos()).x();
            int sceneWidth=this->getScene()->width();
            if(xpos>sceneWidth*0.10&&sceneWidth*0.80){
                float x = (xpos-sceneWidth*0.10)/0.7/sceneWidth*(curveXMax-curveXMin)+curveXMin;
                QStandardItemModel*model = new QStandardItemModel(dataDetailsView);
                if(membership->dataType==solim::CATEGORICAL){
                    float y = membership->getOptimality(int(x+0.5));
                    model->setItem(0,0, new QStandardItem("Covariate value: "+QString::number(int(x+0.5))));
                    model->setItem(1,0, new QStandardItem("Membership value: "+QString::number(int(y))));
                } else {
                    float y = membership->getOptimality(x);
                    model->setItem(0,0, new QStandardItem("Covariate value: "+QString::number(x)));
                    model->setItem(1,0, new QStandardItem("Membership value: "+QString::number(y)));
                }
                dataDetailsView->setModel(model);
                dataDetailsView->resizeColumnsToContents();

            }
        }
    }
}

void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent *e){
    if(editFreehandRule){
        QPointF pt = mapToScene(e->pos());
        int sceneWidth = scene->width();
        int sceneHeight = scene->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        if(pt.x()<xStart||pt.x()>xStart+graphWidth) return;
        double x = 1.0*(pt.x()-xStart)/graphWidth;
        x = x*(curveXMax-curveXMin)+curveXMin;
        double y = 1.0*(yEnd-pt.y())/graphHeight;
        int py=pt.y();
        if(y>1){
            y=1;
            py=yEnd;
        }
        if(y<0){
            y=0;
            py=yEnd-sceneHeight;
        }
        for(int i = 0;i<knotX.size();i++){
            int knotXPt = (knotX[i]-curveXMin)/(curveXMax-curveXMin)*graphWidth+xStart;
            int knotYPt = yEnd - knotY[i]*graphHeight;
            if(fabs(knotXPt-pt.x())<offsetSize){
                if(fabs(knotYPt-pt.y())<offsetSize){
                    knotX.erase(knotX.begin()+i);
                    knotY.erase(knotY.begin()+i);
                    knotX.shrink_to_fit();
                    knotY.shrink_to_fit();
                    emit addFreehandPoint();
                }
                return;
            }
        }
        knotX.push_back(x);
        knotY.push_back(y);
        emit addFreehandPoint();
    } else if(editEnumRule){
        QPointF pt = mapToScene(e->pos());
        int sceneWidth = scene->width();
        int sceneHeight = scene->height();
        int graphWidth = 0.7*sceneWidth;
        int graphHeight = 0.7*sceneHeight;
        int xStart = 0.10*sceneWidth;
        int yEnd = 0.85*sceneHeight;
        if(pt.x()<xStart||pt.x()>xStart+graphWidth) return;
        double x = 1.0*(pt.x()-xStart)/graphWidth;
        double y = 1.0*(yEnd-pt.y())/graphHeight;
        int py=pt.y();
        if(y>1||y<0) return;
        x = x * (curveXMax-curveXMin) + curveXMin;
        for(int i = 0;i<knotX.size();i++){
            int knotXPt = (knotX[i]-curveXMin)/(curveXMax-curveXMin)*graphWidth+xStart;
            if(fabs(knotXPt-pt.x())<offsetSize){
                knotX.erase(knotX.begin()+i);
                knotY.erase(knotY.begin()+i);
                knotX.shrink_to_fit();
                knotY.shrink_to_fit();
                emit addEnumPoint();
                return;
            }
        }
        for(int i = 0; i<knotX.size();i++){
            if(knotX[i]==int(x+0.5))
                return;
        }
        knotX.push_back(int(x+0.5));
        knotY.push_back(1);
        emit addEnumPoint();
    }
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *e){
    moveKnotNum=-1;
}
