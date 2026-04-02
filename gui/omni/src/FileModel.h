#ifndef FILE_MODEL_H
#define FILE_MODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <vector>
#include <filesystem>
#include "fo/core/engine.hpp"

class FileModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY pathChanged)

public:
    enum FileRoles {
        NameRole = Qt::UserRole + 1,
        SizeRole,
        TypeRole,
        DateModifiedRole,
        IconRole,
        IsDirectoryRole,
        IsDuplicateRole
    };

    FileModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentPath() const { return m_currentPath; }
    void setCurrentPath(const QString &path);

    Q_INVOKABLE void openFolder(const QString &path);
    Q_INVOKABLE void goUp();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void findDuplicates();

signals:
    void pathChanged();
    void duplicateDetectionStarted();
    void duplicateDetectionFinished(int groupsFound);

private:
    QString m_currentPath;
    struct FileEntry {
        QString name;
        qint64 size;
        QString type;
        QString dateModified;
        QString icon;
        bool isDirectory;
        bool isDuplicate;
    };
    std::vector<FileEntry> m_files;
    std::unique_ptr<fo::core::Engine> m_engine;

    void loadPath(const std::filesystem::path &path);
};

#endif // FILE_MODEL_H
