#include "NativeUiBootstrap.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QUrl>

#include "NativeUiRuntime.hpp"
#include "OmniQmlRegistration.hpp"

namespace fo::gui {
namespace {

ObjectCreatedHandler create_root_object_failure_handler(const QUrl &main_qml)
{
    return [main_qml](QObject *object, const QUrl &object_url) {
        if (!object && object_url == main_qml) {
            QCoreApplication::exit(-1);
        }
    };
}

} // namespace

int run_omni_shell(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    register_omni_qml_types();

    auto runtime = create_default_declarative_ui_runtime();
    const QUrl main_qml(QStringLiteral("qrc:/main.qml"));

    runtime->set_object_created_handler(create_root_object_failure_handler(main_qml));
    runtime->load(main_qml);
    return app.exec();
}

} // namespace fo::gui
