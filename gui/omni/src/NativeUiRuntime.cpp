#include "NativeUiRuntime.hpp"

#include <QQmlApplicationEngine>
#include <QMetaObject>

#include <utility>

namespace fo::gui {
namespace {

class QtQmlDeclarativeUiRuntime final : public DeclarativeUiRuntime {
public:
    void set_object_created_handler(ObjectCreatedHandler handler) override {
        object_created_handler_ = std::move(handler);

        QObject::connect(
            &engine_,
            &QQmlApplicationEngine::objectCreated,
            &engine_,
            [this](QObject *object, const QUrl &object_url) {
                if (object_created_handler_) {
                    object_created_handler_(object, object_url);
                }
            },
            Qt::QueuedConnection);
    }

    void load(const QUrl &source) override {
        engine_.load(source);
    }

private:
    QQmlApplicationEngine engine_;
    ObjectCreatedHandler object_created_handler_;
};

} // namespace

std::unique_ptr<DeclarativeUiRuntime> create_default_declarative_ui_runtime()
{
    return std::make_unique<QtQmlDeclarativeUiRuntime>();
}

} // namespace fo::gui
