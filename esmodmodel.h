#ifndef ESMODMODEL_H
#define ESMODMODEL_H

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "esmodelement.h"
#include "asyncjsonwriter.h"

class ESModModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ModRoles
    {
        TitleRole = Qt::UserRole + 1,
        StatusRole,
        LangsRole,
        UriRole,
        InfoUriRole,
        PathRole,
        FilesRole,
        StateRole,
        ProgressRole,
        SizeRole,
        TimestampRole,
        GuiBlockedRole,
        MyLikeMarkRole,
        LikeMarksCountRole,
        DislikeMarksCountRole
    };

    enum SortMode {
        AsServer   = 0,
        ByNameUp   = 1,
        ByNameDown = 2,
        BySizeUp   = 3,
        BySizeDown = 4,
        ByDateUp   = 5,
        ByDateDown = 6
    };
    Q_ENUMS(SortMode)

    ESModModel(QObject *parent = 0);
    virtual ~ESModModel();

    void addModElement(ESModElement *element);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void appTitleReceived(const QString &text);
    void appHelpReceived(const QString &text);
    void esIndexReceived();
    void listSorted(SortMode m);

public slots:
    void ESModIndexDownloaded();
    void ESModIndexError(QNetworkReply::NetworkError code);

    void Download(int ind);
    void Abort(int ind);
    void Retry(int ind);
    void Update(int ind);
    void Delete(int ind);

    void elementChanged();
    void elementNeedRemove();

    void SaveLocalModsDB();

    void sortList(SortMode m);
    void filterByKeywords(QString str);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    bool LoadLocalModsDB(QList<ESModElement *> &l);
    void ReindexElements();

    QNetworkAccessManager m_NetMgr;
    AsyncJsonWriter m_JsonWriter;

    QList<ESModElement *> m_initialElements;
    QList<ESModElement *> m_elements;

    SortMode m_lastSortMode;
};

#endif // ESMODMODEL_H