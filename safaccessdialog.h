#ifndef SAFACCESSDIALOG_H
#define SAFACCESSDIALOG_H

#include <QDialog>

namespace Ui {
class SafAccessDialog;
}

class SafAccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SafAccessDialog(QWidget *parent = nullptr);
    ~SafAccessDialog();

private:
    Ui::SafAccessDialog *ui;
};

#endif // SAFACCESSDIALOG_H
