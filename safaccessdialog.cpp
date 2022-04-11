#include "safaccessdialog.h"
#include "ui_safaccessdialog.h"

SafAccessDialog::SafAccessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SafAccessDialog)
{
    ui->setupUi(this);
}

SafAccessDialog::~SafAccessDialog()
{
    delete ui;
}
