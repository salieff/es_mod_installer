#ifndef ESINSTALLEDMODMODEL_H
#define ESINSTALLEDMODMODEL_H

#include <QSortFilterProxyModel>

class ESInstalledModModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ESInstalledModModel(QAbstractItemModel *sModel, bool inv = false, QObject *parent = 0) : QSortFilterProxyModel(parent), inverseFilter(inv)
    {
        setSourceModel(sModel);
    }


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
    void ToggleFavorite(int ind);

private:
    bool inverseFilter;
};


#define DECLARE_NEW_PROXY(class_name) \
    class class_name : public ESInstalledModModel \
    { \
        Q_OBJECT \
    public: \
        class_name(QAbstractItemModel *sModel, QObject *parent = 0) : ESInstalledModModel(sModel, false, parent) {} \
     \
    protected: \
        virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const; \
    };

DECLARE_NEW_PROXY(ESIncompletedModModel)
DECLARE_NEW_PROXY(ESBrokenModModel)
DECLARE_NEW_PROXY(ESFavoriteModModel)
DECLARE_NEW_PROXY(ESReleasedModModel)

#endif // ESINSTALLEDMODMODEL_H
