#include "safaccessdialog.h"
#include "ui_safaccessdialog.h"


SafAccessDialog::SafAccessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SafAccessDialog)
{
    ui->setupUi(this);
    ui->imageLabel->installEventFilter(this);
}

SafAccessDialog::~SafAccessDialog()
{
    delete ui;
}

void SafAccessDialog::adjustImage(void)
{
    QPixmap pixmap;

    if (ui->imageLabel->width() > ui->imageLabel->height())
        pixmap.load(":/icons/saf_access_horiz.png");
    else
        pixmap.load(":/icons/saf_access_vert.png");

    ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

bool SafAccessDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->imageLabel && event->type() == QEvent::Resize)
        adjustImage();

    return QDialog::eventFilter(obj, event);
}
