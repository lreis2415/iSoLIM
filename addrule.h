#ifndef ADDEXPERTBASE_H
#define ADDEXPERTBASE_H

#include <QDialog>
#include <QMessageBox>
#include <QGraphicsTextItem>
#include <vector>
#include "mygraphicsview.h"
#include "project.h"
#include "simpledialog.h"
#include "solim-lib-forqt.h"

namespace Ui {
class AddRule;
}

class AddRule : public QDialog
{
    Q_OBJECT

public:
    explicit AddRule(SoLIMProject *proj, QWidget *parent = nullptr);
    ~AddRule();
signals:
    void createBase(const QString basename);
    void createPrototype(const QString basename, const QString prototypeName);
    void addlayer();
    void updatePrototype();
private slots:
    void on_btn_create_base_clicked();

    void on_btn_add_proto_clicked();

    void on_radioButton_range_clicked();

    void on_radioButton_point_clicked();

    void on_radioButton_freehand_clicked();

    void on_radioButton_enum_clicked();

    void on_comboBox_cov_activated(const QString &arg1);

    void on_btn_add_prop_clicked();

    void on_btn_add_rule_clicked();

    void on_comboBox_curve_activated(const QString &arg1);

    void on_btn_add_opt_val_clicked();

    void on_btn_reset_clicked();

    void onAddFreehandRule(const double x, const double y);

private:
    Ui::AddRule *ui;
    QStringList prototypeNames;
    QString basename;
    SoLIMProject *proj;
    MyGraphicsView *myview;
    vector<int> enumVals;
    int enumMax;
    int enumMin;
    bool enumViewInit;
    vector<double> *freeKnotX;
    vector<double> *freeKnotY;
    void pointRuleWarn();
    void drawMembershipFunction(solim::Curve *c);
    void addSuccess(QString content);
    void drawEnum(int num);
    void drawEnumRange();
    void enumRuleWarn();
};

#endif // ADDEXPERTBASE_H
