#ifndef ITEMSELECTIONWINDOW_H
#define ITEMSELECTIONWINDOW_H

#include <QDialog>

namespace Ui {
class itemSelectionWindow;
}

class itemSelectionWindow : public QDialog
{
    Q_OBJECT

public:
    enum itemSelectionMode {PROTOTYPEBASESELECTION, CATEGORICALPROPERTYSELECTION};
    explicit itemSelectionWindow(itemSelectionMode mode, QStringList names, QString selected, QWidget *parent = nullptr);
    QString selectedNames;
    ~itemSelectionWindow();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::itemSelectionWindow *ui;
};

#endif // EDITPROTOTYPEBASES_H
