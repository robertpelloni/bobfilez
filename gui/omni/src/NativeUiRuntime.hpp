#pragma once

#include <QUrl>

#include <functional>
#include <memory>

class QObject;

namespace fo::gui {

using ObjectCreatedHandler = std::function<void(QObject *, const QUrl &)>;

class DeclarativeUiRuntime {
public:
    virtual ~DeclarativeUiRuntime() = default;

    virtual void set_object_created_handler(ObjectCreatedHandler handler) = 0;
    virtual void load(const QUrl &source) = 0;
};

std::unique_ptr<DeclarativeUiRuntime> create_default_declarative_ui_runtime();

} // namespace fo::gui
