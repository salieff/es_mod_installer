#ifndef ESINSTALLEDMODMODEL_H
#define ESINSTALLEDMODMODEL_H

#include <QSortFilterProxyModel>

class ESInstalledModModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ESInstalledModModel(bool inv = false, QObject *parent = 0);
    virtual ~ESInstalledModModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

public slots:
    void Download(int ind);
    void Abort(int ind);
    void Retry(int ind);
    void Update(int ind);
    void Delete(int ind);
    void SendLike(int ind, int l);
    void ShowError(int ind);

private:
    bool inverseFilter;
};

class ESIncompletedModModel : public ESInstalledModModel
{
    Q_OBJECT
public:
    ESIncompletedModModel(QObject *parent = 0);
    virtual ~ESIncompletedModModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

class ESBrokenModModel : public ESInstalledModModel
{
    Q_OBJECT
public:
    ESBrokenModModel(QObject *parent = 0);
    virtual ~ESBrokenModModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

#endif // ESINSTALLEDMODMODEL_H
