#include "editprototypebases.h"
#include "ui_editprototypebases.h"

editPrototypeBases::editPrototypeBases(QStringList names, QString selected, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editPrototypeBases)
{
    QStringList selectedNameList = selected.split(';');
    ui->setupUi(this);
    QStringListIterator it(names);
    while (it.hasNext())
    {
          QString name=it.next();
          QListWidgetItem *listItem = new QListWidgetItem(name,ui->listWidget);
          if(selectedNameList.contains(name))
              listItem->setCheckState(Qt::Checked);
          else
              listItem->setCheckState(Qt::Unchecked);
          ui->listWidget->addItem(listItem);
    }
    selectedNames=selected;
}

editPrototypeBases::~editPrototypeBases()
{
    delete ui;
}

void editPrototypeBases::on_buttonBox_accepted()
{
    selectedNames="";
    for(int i = 0; i < ui->listWidget->count(); ++i)
    {
        QListWidgetItem* item = ui->listWidget->item(i);
        if(item->checkState()==Qt::Checked){
            selectedNames.append(item->text()).append(';');
        }
    }
    selectedNames=selectedNames.left(selectedNames.length()-1);
}
