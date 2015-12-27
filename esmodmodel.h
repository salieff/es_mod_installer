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
        PathRole,
        FilesRole,
        StateRole,
        ProgressRole,
        SizeRole,
        TimestampRole,
        GuiBlockedRole
    };

    ESModModel(QObject *parent = 0);
    virtual ~ESModModel();

    void setBusyIndicator(QObject *bus);
    void setAppTitleText(QObject *txt);
    void setHelpText(QObject *txt);

    void addModElement(ESModElement *element);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

public slots:
    void ESModIndexDownloaded();
    void ESModIndexError(QNetworkReply::NetworkError code);

    void Download(int ind);
    void Abort(int ind);
    void Retry(int ind);
    void Update(int ind);
    void Delete(int ind);

    void elementChanged(int ind = -1);
    void elementNeedRemove();

    void SaveLocalModsDB();

    void sortAsServer();
    void sortByName(int updown);
    void sortBySize(int updown);
    void sortByDate(int updown);
    void filterByKeywords(QString str);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    bool LoadLocalModsDB(QList<ESModElement *> &l);

    QNetworkAccessManager m_NetMgr;
    AsyncJsonWriter m_JsonWriter;

    QList<ESModElement *> m_initialElements;
    QList<ESModElement *> m_elements;

    QObject *m_busyIndicator;
    QObject *m_appTitleText;
    QObject *m_helpText;
};

#endif // ESMODMODEL_H
