#include "NativeUiLaunchConfig.hpp"

#include <QCoreApplication>
#include <QString>

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

bool NativeUiLaunchConfig::is_valid() const
{
    return !main_qml.isEmpty() && register_types && runtime_factory;
}

NativeUiLaunchConfig create_default_omni_ui_launch_config()
{
    NativeUiLaunchConfig config;
    config.main_qml = QUrl(QStringLiteral("qrc:/main.qml"));
    config.register_types = []() {
        register_omni_qml_types();
    };
    config.runtime_factory = []() {
        return create_default_declarative_ui_runtime();
    };
    config.object_created_handler = create_root_object_failure_handler(config.main_qml);
    return config;
}

} // namespace fo::gui
