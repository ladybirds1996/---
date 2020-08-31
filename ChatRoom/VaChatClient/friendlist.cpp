#include "friendlist.h"
#include "ui_friendlist.h"

friendlist::friendlist(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::friendlist)
{
    ui->setupUi(this);
}

friendlist::~friendlist()
{
    delete ui;
}
