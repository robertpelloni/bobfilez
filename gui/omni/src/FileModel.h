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
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)

public:
    enum FileRoles {
        NameRole = Qt::UserRole + 1,
        SizeRole,
        TypeRole,
        DateModifiedRole,
        IconRole,
        IsDirectoryRole,
        IsDuplicateRole,
        IsSelectedRole
    };

    FileModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentPath() const { return m_currentPath; }
    void setCurrentPath(const QString &path);
    
    int selectedCount() const { return m_selectedCount; }

    Q_INVOKABLE void openFolder(const QString &path);
    Q_INVOKABLE void goUp();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void findDuplicates();
    Q_INVOKABLE void setSearchFilter(const QString &filter);
    Q_INVOKABLE void sortBy(const QString &column, bool ascending);
    Q_INVOKABLE void toggleSelection(int index);
    Q_INVOKABLE void clearSelection();

signals:
    void pathChanged();
    void selectedCountChanged();
    void duplicateDetectionStarted();
    void duplicateDetectionFinished(int groupsFound);

private:
    QString m_currentPath;
    QString m_searchFilter;
    int m_selectedCount = 0;
    
    struct FileEntry {
        QString name;
        qint64 size;
        QString type;
        QString dateModified;
        QString icon;
        bool isDirectory;
        bool isDuplicate;
        bool isSelected = false;
    };
    std::vector<FileEntry> m_allFiles; // Unfiltered list
    std::vector<FileEntry> m_files;    // Filtered/Displayed list
    std::unique_ptr<fo::core::Engine> m_engine;

    void loadPath(const std::filesystem::path &path);
    void updateSelectedCount();
};

#endif // FILE_MODEL_H
