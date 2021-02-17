#ifndef EDITPROTOTYPEBASES_H
#define EDITPROTOTYPEBASES_H

#include <QDialog>

namespace Ui {
class editPrototypeBases;
}

class editPrototypeBases : public QDialog
{
    Q_OBJECT

public:
    explicit editPrototypeBases(QStringList names, QString selected, QWidget *parent = nullptr);
    QString selectedNames;
    ~editPrototypeBases();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::editPrototypeBases *ui;
};

#endif // EDITPROTOTYPEBASES_H
