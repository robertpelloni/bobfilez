/// @file TreemapModel.cpp
/// @brief Implementation of the Treemap QML bridge.

#include "TreemapModel.h"
#include "fo/core/registry.hpp"
#include <QDebug>

namespace fo::gui {

TreemapModel::TreemapModel(QObject* parent) : QAbstractListModel(parent) {}

int TreemapModel::rowCount(const QModelIndex& parent) const {
    return static_cast<int>(flattened_nodes_.size());
}

QVariant TreemapModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= flattened_nodes_.size()) return {};
    auto* node = flattened_nodes_[index.row()];

    switch (role) {
        case NameRole: return QString::fromStdString(node->name);
        case SizeRole: return static_cast<qlonglong>(node->size);
        case PathRole: return QString::fromStdString(node->path.string());
        case TypeRole: return QString::fromStdString(node->type);
        case RectXRole: return node->x;
        case RectYRole: return node->y;
        case RectWRole: return node->width;
        case RectHRole: return node->height;
    }
    return {};
}

QHash<int, QByteArray> TreemapModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[SizeRole] = "size";
    roles[PathRole] = "path";
    roles[TypeRole] = "type";
    roles[RectXRole] = "rx";
    roles[RectYRole] = "ry";
    roles[RectWRole] = "rw";
    roles[RectHRole] = "rh";
    return roles;
}

void TreemapModel::setRootPath(const QString& path) {
    if (root_path_ != path) {
        root_path_ = path;
        emit rootPathChanged();
        refresh();
    }
}

void TreemapModel::refresh() {
    is_busy_ = true;
    emit isBusyChanged();

    auto engine = fo::core::Registry<fo::core::ITreemapEngine>::instance().create("default");
    if (!engine) return;

    beginResetModel();
    root_node_ = engine->build_tree(root_path_.toStdString());
    engine->calculate_layout(root_node_, 0, 0, 1.0, 1.0); // Normalized coordinates
    
    flattened_nodes_.clear();
    flatten(root_node_);
    endResetModel();

    is_busy_ = false;
    emit isBusyChanged();
}

void TreemapModel::flatten(fo::core::TreemapNode& node) {
    if (node.children.empty() || node.path.depth() > 3) {
        flattened_nodes_.push_back(&node);
    } else {
        for (auto& child : node.children) flatten(child);
    }
}

} // namespace fo::gui
