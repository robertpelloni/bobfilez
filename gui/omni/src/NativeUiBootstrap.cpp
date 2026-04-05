#include "NativeUiBootstrap.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QMetaObject>
#include <QUrl>

#include <functional>
#include <memory>
#include <utility>

#include "FileModel.h"
#include "TreemapModel.h"
#include "NativeMarkdownView.h"

namespace fo::gui {
namespace {

using ObjectCreatedHandler = std::function<void(QObject *, const QUrl &)>;

class DeclarativeUiRuntime {
public:
    virtual ~DeclarativeUiRuntime() = default;

    virtual void set_object_created_handler(ObjectCreatedHandler handler) = 0;
    virtual void load(const QUrl &source) = 0;
};

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

std::unique_ptr<DeclarativeUiRuntime> create_default_declarative_ui_runtime()
{
    return std::make_unique<QtQmlDeclarativeUiRuntime>();
}

void register_omni_qml_types()
{
    qmlRegisterType<FileModel>("Omni.File", 1, 0, "FileModel");
    qmlRegisterType<fo::gui::TreemapModel>("Omni.Viz", 1, 0, "TreemapModel");
    qmlRegisterType<fo::gui::NativeMarkdownView>("Omni.Native", 1, 0, "MarkdownView");
}

} // namespace

int run_omni_shell(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    register_omni_qml_types();

    auto runtime = create_default_declarative_ui_runtime();
    const QUrl main_qml(QStringLiteral("qrc:/main.qml"));

    runtime->set_object_created_handler(
        [main_qml](QObject *object, const QUrl &object_url) {
            if (!object && object_url == main_qml) {
                QCoreApplication::exit(-1);
            }
        });

    runtime->load(main_qml);
    return app.exec();
}

} // namespace fo::gui
