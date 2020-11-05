#include "changecovname.h"
#include "ui_changecovname.h"

changeCovName::changeCovName(solim::Prototype *proto, QWidget *parent) :
    QDialog(parent),proto(proto),
    ui(new Ui::changeCovName)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem("Current name"));
    ui->tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem("New name"));
    ui->tableWidget->setRowCount(proto->envConditionSize);
    ui->tableWidget->verticalHeader()->setVisible(false);
    for(int i = 0; i<proto->envConditionSize;i++){
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(proto->envConditions[i].covariateName.c_str()));
        QLineEdit *newCovName = new QLineEdit();
        newCovName->setText(proto->envConditions[i].covariateName.c_str());
        ui->tableWidget->setCellWidget(i,1,newCovName);
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(proto->envConditions[i].covariateName.c_str()));
    }
    ui->tableWidget->update();
}

changeCovName::~changeCovName()
{
    delete ui;
}

void changeCovName::on_cancel_btn_clicked()
{
    return;
}

void changeCovName::on_ok_btn_clicked()
{
    for(int i = 0; i<proto->envConditionSize;i++){
        string newName = ((QLineEdit*) ui->tableWidget->cellWidget(i,1))->text().toStdString();
        if(newName!=proto->envConditions[i].covariateName){
            isChanged=true;
            proto->envConditions[i].covariateName=newName;
        }
    }
    close();
    return;
}
