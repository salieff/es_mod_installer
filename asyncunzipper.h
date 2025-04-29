#ifndef ASYNCUNZIPPER_H
#define ASYNCUNZIPPER_H

#include <QThread>
#include <set>
#include <map>
#include <atomic>
#include "minizip/unzip.h"


class AsyncUnzipper : public QThread
{
    Q_OBJECT
public:
    explicit AsyncUnzipper(QObject * parent = 0);

    bool unzip(QString zipfile);
    bool aborted();
    bool failed();
    QString errorString();
    QStringList unpackedFiles();

signals:
    void progress(int p);

public slots:
    void abort();

protected:
    virtual void run();

private:
    constexpr static bool ESTIMATE = true;
    constexpr static bool UNPACK = false;

    char *m_unpackZipBuffer = nullptr;
    bool unpackZip(bool estimateOnly = UNPACK);
    bool saveCurrentUnpFile(unzFile ufd, QString fname);

    QString m_zipFile;
    qint64 m_totalSize;
    qint64 m_unpackedSize;
    int m_progress;
    QStringList m_unpackedFiles;

    std::atomic_bool m_abortFlag;
    std::atomic_bool m_failedFlag;
    QString m_errorString;

    std::set<QString> m_topFolders;
    QStringList m_topFiles;
    void deletePreviousInstallation();

    struct FolderTreeElement {
        std::map<QString, FolderTreeElement> children;

        void clear();
        void addFoldersList(const QStringList &foldersList);
        bool createFolderTree(QString &errorString, const QString &rootFolder = "") const;
    } m_foldersToCreate;
};

#endif // ASYNCUNZIPPER_H
