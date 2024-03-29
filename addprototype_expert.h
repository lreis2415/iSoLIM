#ifndef ADDEXPERTBASE_H
#define ADDEXPERTBASE_H

#include <QDialog>
#include <QMessageBox>
#include <QGraphicsTextItem>
#include <QLayout>
#include <vector>
#include "mygraphicsview.h"
#include "project.h"
#include "simpledialog.h"

namespace Ui {
class AddPrototype_Expert;
}

class AddPrototype_Expert : public QDialog
{
    Q_OBJECT

public:
    explicit AddPrototype_Expert(SoLIMProject *proj, int protoNum, string currentBaseName="", QWidget *parent = nullptr);
    ~AddPrototype_Expert();
signals:
    void addlayer();
    void updatePrototype();
private slots:
    void on_radioButton_range_toggled(bool checked);

    void on_radioButton_freehand_toggled(bool checked);

    void on_radioButton_enum_toggled(bool checked);

    void on_comboBox_cov_activated(const QString &arg1);

    void on_btn_add_prop_clicked();

    void on_btn_add_rule_clicked();

    void on_comboBox_curve_activated(const QString &arg1);

    void on_btn_add_opt_val_clicked();

    void on_btn_reset_clicked();

    void onAddFreehandPoint();

    void onAddEnumPoint();

    void on_lineEdit_min_cov_textChanged(const QString &arg1);

    void on_lineEdit_max_cov_textChanged(const QString &arg1);

    void on_comboBox_datatype_activated(int index);

    void on_comboBox_datatype_currentIndexChanged(int index);

    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
    };
    void on_btn_create_clicked();

    void on_lineEdit_lc_textChanged(const QString &arg1);

    void on_lineEdit_lu_textChanged(const QString &arg1);

    void on_lineEdit_hu_textChanged(const QString &arg1);

    void on_lineEdit_hc_textChanged(const QString &arg1);

    void on_comboBox_curve_activated(int index);

private:
    Ui::AddPrototype_Expert *ui;
    QStringList prototypeNames;
    QString basename;
    SoLIMProject *proj;
    int protoNum;
    MyGraphicsView *myview;
    vector<int> enumVals;
    float rangeMax;
    float rangeMin;
    bool enumViewInit;
    vector<double> *freeKnotX;
    vector<double> *freeKnotY;
    string currentBasename;
    void drawMembershipFunction(solim::Curve *c);
    void drawEnumRange();
    bool getPointRule(solim::Curve &c, bool &warn);
    void warn(QString msg){
        QMessageBox qb;
        qb.setText(msg);
        qb.exec();
    }

};

#endif // ADDEXPERTBASE_H
