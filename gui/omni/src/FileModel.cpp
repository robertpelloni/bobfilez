#include "FileModel.h"
#include <QDateTime>
#include <QDir>
#include <chrono>
#include <iomanip>
#include <sstream>
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
        case IsSelectedRole: return file.isSelected;
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
    roles[IsSelectedRole] = "isSelected";
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

void FileModel::setSearchFilter(const QString &filter) {
    if (m_searchFilter != filter) {
        m_searchFilter = filter;
        
        beginResetModel();
        m_files.clear();
        for (const auto& f : m_allFiles) {
            if (filter.isEmpty() || f.name.contains(filter, Qt::CaseInsensitive)) {
                m_files.push_back(f);
            }
        }
        endResetModel();
    }
}

void FileModel::sortBy(const QString &column, bool ascending) {
    beginResetModel();
    auto sortFunc = [&](const FileEntry &a, const FileEntry &b) {
        bool res = false;
        if (a.isDirectory != b.isDirectory) {
            res = a.isDirectory;
            return ascending ? res : !res;
        }
        
        if (column == "Name") res = a.name.toLower() < b.name.toLower();
        else if (column == "Size") res = a.size < b.size;
        else if (column == "Type") res = a.type.toLower() < b.type.toLower();
        else if (column == "Date modified") res = a.dateModified < b.dateModified;
        
        return ascending ? res : !res;
    };
    
    std::sort(m_allFiles.begin(), m_allFiles.end(), sortFunc);
    
    // Re-apply filter
    m_files.clear();
    for (const auto& f : m_allFiles) {
        if (m_searchFilter.isEmpty() || f.name.contains(m_searchFilter, Qt::CaseInsensitive)) {
            m_files.push_back(f);
        }
    }
    endResetModel();
}

void FileModel::toggleSelection(int index) {
    if (index >= 0 && index < m_files.size()) {
        m_files[index].isSelected = !m_files[index].isSelected;
        
        // Sync back to all files if needed (for persistence across filter changes)
        for (auto& f : m_allFiles) {
            if (f.name == m_files[index].name) {
                f.isSelected = m_files[index].isSelected;
                break;
            }
        }
        
        emit dataChanged(this->index(index), this->index(index), {IsSelectedRole});
        updateSelectedCount();
    }
}

void FileModel::clearSelection() {
    for (auto& f : m_files) f.isSelected = false;
    for (auto& f : m_allFiles) f.isSelected = false;
    emit dataChanged(index(0), index(m_files.size() - 1), {IsSelectedRole});
    updateSelectedCount();
}

void FileModel::updateSelectedCount() {
    int count = 0;
    for (const auto& f : m_files) if (f.isSelected) count++;
    if (m_selectedCount != count) {
        m_selectedCount = count;
        emit selectedCountChanged();
    }
}

void FileModel::findDuplicates() {
    emit duplicateDetectionStarted();
    
    std::vector<std::filesystem::path> roots = { m_currentPath.toStdString() };
    auto files = m_engine->scan(roots, {}, false, false);
    auto groups = m_engine->find_duplicates(files);
    
    std::set<std::string> duplicatePaths;
    for (const auto& group : groups) {
        for (const auto& file : group.files) {
            duplicatePaths.insert(file.path.string());
        }
    }
    
    for (auto& entry : m_allFiles) {
        std::filesystem::path fullPath = std::filesystem::path(m_currentPath.toStdString()) / entry.name.toStdString();
        entry.isDuplicate = duplicatePaths.count(fullPath.string()) > 0;
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
    m_allFiles.clear();
    m_files.clear();

    try {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            FileEntry f;
            f.name = QString::fromStdString(entry.path().filename().string());
            f.isDirectory = entry.is_directory();
            f.isDuplicate = false;
            f.isSelected = false;
            
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
            
            std::ostringstream ts;
#ifdef _WIN32
            std::tm tm_buf;
            localtime_s(&tm_buf, &t);
            ts << std::put_time(&tm_buf, "%m/%d/%Y %I:%M %p");
#else
            ts << std::put_time(std::localtime(&t), "%m/%d/%Y %I:%M %p");
#endif
            f.dateModified = QString::fromStdString(ts.str());

            m_allFiles.push_back(f);
        }
        
        std::sort(m_allFiles.begin(), m_allFiles.end(), [](const FileEntry &a, const FileEntry &b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory;
            return a.name.toLower() < b.name.toLower();
        });

        m_files = m_allFiles;

    } catch (const std::exception &e) {
        qDebug() << "Error loading path:" << e.what();
    }

    m_selectedCount = 0;
    emit selectedCountChanged();
    endResetModel();
}
