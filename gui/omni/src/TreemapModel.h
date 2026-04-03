#pragma once
/// @file treemap_model.hpp
/// @brief QAbstractItemModel for the visual Treemap (Topology map).
///
/// Maps the C++ TreemapEngine output to a format QML can consume
/// efficiently to render the nested rectangle grid.

#include <QAbstractListModel>
#include "fo/core/treemap_engine_interface.hpp"

namespace fo::gui {

class TreemapModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString rootPath READ rootPath WRITE setRootPath NOTIFY rootPathChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        SizeRole,
        PathRole,
        TypeRole,
        RectXRole,
        RectYRole,
        RectWRole,
        RectHRole
    };

    explicit TreemapModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString rootPath() const { return root_path_; }
    void setRootPath(const QString& path);
    bool isBusy() const { return is_busy_; }

    Q_INVOKABLE void refresh();

signals:
    void rootPathChanged();
    void isBusyChanged();

private:
    QString root_path_;
    bool is_busy_ = false;
    fo::core::TreemapNode root_node_;
    std::vector<fo::core::TreemapNode*> flattened_nodes_; // Only leaf nodes or first level for visualization

    void flatten(fo::core::TreemapNode& node);
};

} // namespace fo::gui
