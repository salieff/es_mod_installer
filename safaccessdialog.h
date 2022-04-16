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

public slots:
    void adjustImage(void);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::SafAccessDialog *ui;
};

#endif // SAFACCESSDIALOG_H
