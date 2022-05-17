#include "itemselectionwindow.h"
#include "ui_itemselectionwindow.h"

itemSelectionWindow::itemSelectionWindow(itemSelectionMode mode, QStringList names, QString selected, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::itemSelectionWindow)
{
    ui->setupUi(this);
    QStringList selectedNameList = selected.split(';');
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

    if(mode==PROTOTYPEBASESELECTION){
        setWindowTitle("Select prototype base used for inference");
    }
    if(mode==CATEGORICALPROPERTYSELECTION){
        setWindowTitle("Select property treated as categorical data");
    }

}

itemSelectionWindow::~itemSelectionWindow()
{
    delete ui;
}

void itemSelectionWindow::on_buttonBox_accepted()
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
