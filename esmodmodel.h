#ifndef ESMODMODEL_H
#define ESMODMODEL_H

#include <QAbstractListModel>
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
    Q_ENUMS(SortMode)

    ESModModel(QObject *parent = 0);
    virtual ~ESModModel();

    void addModElement(ESModElement *element);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    static QString ESModsFolder();
    static QString ESTracebackFileName(bool forLog = false);

#ifdef Q_OS_IOS
    static QString ESFolderForIOS(QStringList &dirs);
    static QString ESTraceFolderForIOS(QStringList &dirs);
#elif defined(ANDROID)
    static QString ESFolderForAndroid(QStringList &dirs);
#endif

signals:
    void appTitleReceived(const QString &text);
    void appHelpReceived(const QString &text, bool fromServer = true);
    void esIndexReceived();
    void listSorted(SortMode m);
    void currentModsFolder(QString newFolder);
    void tracebackText(QString text);
    void balloonText(QString text);
    void showMeHelp(QString chapter);

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
    void changeModsFolder(QString f);
    void resetModsFolder();

    void copyTraceback(bool forLog = false);

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

    SortMode m_lastSortMode;
    QString m_helpText;

    static QString m_ESModsFolder;
    static QString m_CustomUserModsFolder;
    static QString m_FolderFoundDebugLogString;
#ifdef Q_OS_IOS
    static QString m_traceFolderForIos;
#endif
};

#endif // ESMODMODEL_H
