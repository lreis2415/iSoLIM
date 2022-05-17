#ifndef LEGENDWINDOW_H
#define LEGENDWINDOW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsTextItem>

namespace Ui {
class legendwindow;
}

class LegendWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LegendWindow(QWidget *parent = nullptr);
    ~LegendWindow();
    void updateview(double max, double min);
private:
    Ui::legendwindow *ui;
    QGraphicsView *legendView;
};

#endif // LEGENDWINDOW_H
