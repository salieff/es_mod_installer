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
        FavoriteRole,
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
        ByStatus         = 1,
        ByNameUp         = 2,
        ByNameDown       = 3,
        BySizeUp         = 4,
        BySizeDown       = 5,
        ByDateUp         = 6,
        ByDateDown       = 7,
        ByScore          = 8,
        ByVotesCount     = 9,
        ByActiveInstalls = 10,
        ByTotalInstalls  = 11,
        ByLifeTime       = 12
    };
    Q_ENUM(SortMode)

    ESModModel(QObject *parent = 0);
    virtual ~ESModModel();

    void addModElement(ESModElement *element);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    const QStringList & LocalFiles(int ind) const;

signals:
    void appTitleReceived(const QString &text);
    void appHelpReceived(const QString &text, bool fromServer = true);
    void esIndexReceived();
    void listSorted(SortMode m);
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
    void ToggleFavorite(int ind);

    void elementChanged();
    void elementNeedRemove();

    void SaveLocalModsDB();

    void sortList(SortMode m);
    void filterByKeywords(QString str);

    void helpRead(QString str);
    void copyTraceback(bool forLog = false);
    void copyToClipboard(const QString &txt, const QString &msg);

private slots:
    void showDefferedHelp();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void requestAllLikes();
    void requestAllStatistics();
    bool LoadLocalModsDB(QList<ESModElement *> &l);
    void ReindexElements();

    AsyncJsonWriter m_JsonWriter;

    QList<ESModElement *> m_initialElements;
    QList<ESModElement *> m_elements;

    SortMode m_lastSortMode = AsServer;
    QString m_helpText;

    bool m_needShowHelp = false;
};

#endif // ESMODMODEL_H
