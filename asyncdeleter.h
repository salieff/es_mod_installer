#ifndef ASYNCDELETER_H
#define ASYNCDELETER_H

#include <QThread>


class AsyncDeleter : public QThread
{
    Q_OBJECT
public:
    explicit AsyncDeleter(QObject *parent = NULL);
    bool deleteFiles(QStringList flist);

signals:
    void progress(int p) const;

protected:
    virtual void run();

private:
    QStringList m_localFiles;

    struct FolderTreeElement
    {
        FolderTreeElement(const AsyncDeleter *deleter, qint64 *totalElements, qint64 *remainsElements, int *progress);

        QStringList m_files;
        std::map<QString, FolderTreeElement> m_folders;

        const AsyncDeleter *m_deleter = nullptr;
        qint64 *m_totalElements = nullptr;
        qint64 *m_remainsElements = nullptr;
        int *m_progress = nullptr;

        void addFilePath(const QString &path);
        void addFilePathAsList(const QStringList &foldersList, const QString &fname);
        void deleteFolderTree(const QString &rootFolder = "");
        void decreaseProgress();
    };
};

#endif // ASYNCDELETER_H
