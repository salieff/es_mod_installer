#ifndef ESMODMODEL_H
#define ESMODMODEL_H

#include <QAbstractListModel>
#include <QNetworkReply>

#include "esmodelement.h"
#include "asyncjsonwriter.h"
#include "modpaths.h"


class ESModModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ModRoles
    {
        TitleRole = Qt::UserRole + 1,
        StatusRole,
        LangsRole,
        InfoUriRole,
        FilesRole,
        StateRole,
        ProgressRole,
        SizeRole,
        TimestampRole,
        GuiBlockedRole,
        MyLikeMarkRole,
        LikeMarksCountRole,
        DislikeMarksCountRole,
        InstallsTotalRole,
        InstallsActiveRole,
        InstallsTotalMonthRole,
        InstallsActiveMonthRole,
        InstallsTotalWeekRole,
        InstallsActiveWeekRole,
        LifeTimeAVGRole,
        LifeTimeMAXRole
    };

    enum SortMode {
        AsServer         = 0,
        ByNameUp         = 1,
        ByNameDown       = 2,
        BySizeUp         = 3,
        BySizeDown       = 4,
        ByDateUp         = 5,
        ByDateDown       = 6,
        ByScore          = 7,
        ByVotesCount     = 8,
        ByActiveInstalls = 9,
        ByTotalInstalls  = 10,
        ByLifeTime       = 11
    };
    Q_ENUM(SortMode)

    enum ModsInstallLocation {
        ModsInstallLocationData  = 0,
        ModsInstallLocationMedia = 1
    };
    Q_ENUM(ModsInstallLocation)

    ESModModel(QObject *parent = 0);
    virtual ~ESModModel();

    void addModElement(ESModElement *element);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void appTitleReceived(const QString &text);
    void appHelpReceived(const QString &text, bool fromServer = true);
    void esIndexReceived();
    void listSorted(SortMode m);
    void tracebackText(QString text);
    void balloonText(QString text);
    void showMeHelp(QString chapter);
    void currentModsInstallLocation(ModsInstallLocation location);

public slots:
    void ESModIndexDownloaded();
    void ESModIndexError(QNetworkReply::NetworkError code);
    void AllLikesReceived();
    void AllStatisticsReceived();

    void Download(int ind);
    void Abort(int ind);
    void Retry(int ind);
    void Update(int ind);
    void Delete(int ind);
    void SendLike(int ind, int l);
    void ShowError(int ind);

    void elementChanged();
    void elementNeedRemove();

    void SaveLocalModsDB();

    void sortList(SortMode m);
    void filterByKeywords(QString str);

    void helpRead(QString str);
    void copyTraceback(bool forLog = false);

    void setModsInstallLocation(ModsInstallLocation location);

private slots:
    void showDefferedHelp();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void requestAllLikes();
    void requestAllStatistics();
    bool LoadLocalModsDB(QList<ESModElement *> &l);
    void ReindexElements();

    void copyToClipboard(QString &txt, QString msg);

    AsyncJsonWriter m_JsonWriter;

    QList<ESModElement *> m_initialElements;
    QList<ESModElement *> m_elements;

    SortMode m_lastSortMode = AsServer;
    QString m_helpText;

    QString m_ESModsFolder = ANDROID_ES_MODS_FOLDER_DATA;
    bool m_needShowHelp = false;
    ModsInstallLocation m_ModsInstallLocation = ModsInstallLocationData;
};

#endif // ESMODMODEL_H
