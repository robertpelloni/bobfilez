#include "FileModel.h"
#include <QDateTime>
#include <QDir>
#include <chrono>
#include "fo/core/export.hpp"

FileModel::FileModel(QObject *parent) : QAbstractListModel(parent) {
    fo::core::EngineConfig cfg;
    cfg.db_path = "omni_file.db";
    m_engine = std::make_unique<fo::core::Engine>(cfg);
    
    openFolder(QDir::homePath());
}

int FileModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_files.size());
}

QVariant FileModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_files.size()) return QVariant();

    const auto &file = m_files[index.row()];
    switch (role) {
        case NameRole: return file.name;
        case SizeRole: return file.isDirectory ? "" : QString::fromStdString(fo::core::Exporter::format_size(file.size));
        case TypeRole: return file.type;
        case DateModifiedRole: return file.dateModified;
        case IconRole: return file.icon;
        case IsDirectoryRole: return file.isDirectory;
        case IsDuplicateRole: return file.isDuplicate;
        default: return QVariant();
    }
}

QHash<int, QByteArray> FileModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "fileName";
    roles[SizeRole] = "fileSize";
    roles[TypeRole] = "fileType";
    roles[DateModifiedRole] = "dateModified";
    roles[IconRole] = "fileIcon";
    roles[IsDirectoryRole] = "isDirectory";
    roles[IsDuplicateRole] = "isDuplicate";
    return roles;
}

void FileModel::setCurrentPath(const QString &path) {
    if (m_currentPath != path) {
        m_currentPath = path;
        emit pathChanged();
        loadPath(std::filesystem::path(path.toStdString()));
    }
}

void FileModel::openFolder(const QString &path) {
    setCurrentPath(path);
}

void FileModel::goUp() {
    std::filesystem::path p(m_currentPath.toStdString());
    if (p.has_parent_path()) {
        openFolder(QString::fromStdString(p.parent_path().string()));
    }
}

void FileModel::refresh() {
    loadPath(std::filesystem::path(m_currentPath.toStdString()));
}

void FileModel::findDuplicates() {
    emit duplicateDetectionStarted();
    
    // Perform scan of current folder (non-recursive for now)
    std::vector<std::filesystem::path> roots = { m_currentPath.toStdString() };
    auto files = m_engine->scan(roots, {}, false, false);
    
    // Find duplicates
    auto groups = m_engine->find_duplicates(files);
    
    // Map duplicates back to our model entries
    std::set<std::string> duplicatePaths;
    for (const auto& group : groups) {
        for (const auto& file : group.files) {
            duplicatePaths.insert(file.path.string());
        }
    }
    
    for (auto& entry : m_files) {
        std::filesystem::path fullPath = std::filesystem::path(m_currentPath.toStdString()) / entry.name.toStdString();
        entry.isDuplicate = duplicatePaths.count(fullPath.string()) > 0;
    }
    
    emit dataChanged(index(0), index(m_files.size() - 1), {IsDuplicateRole});
    emit duplicateDetectionFinished(static_cast<int>(groups.size()));
}

void FileModel::loadPath(const std::filesystem::path &path) {
    beginResetModel();
    m_files.clear();

    try {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            FileEntry f;
            f.name = QString::fromStdString(entry.path().filename().string());
            f.isDirectory = entry.is_directory();
            f.isDuplicate = false;
            
            if (!f.isDirectory) {
                f.size = entry.file_size();
                f.type = QString::fromStdString(entry.path().extension().string());
                f.icon = "file";
            } else {
                f.size = 0;
                f.type = "Folder";
                f.icon = "folder";
            }

            auto ftime = entry.last_write_time();
            auto sys_tp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
            auto t = std::chrono::system_clock::to_time_t(sys_tp);
            f.dateModified = QString::fromStdString(std::ctime(&t)).trimmed();

            m_files.push_back(f);
        }
        
        // Sort: Directories first, then names
        std::sort(m_files.begin(), m_files.end(), [](const FileEntry &a, const FileEntry &b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory;
            return a.name.toLower() < b.name.toLower();
        });

    } catch (const std::exception &e) {
        qDebug() << "Error loading path:" << e.what();
    }

    endResetModel();
}
