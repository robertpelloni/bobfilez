#include "NativeUiBootstrap.hpp"

#include <QGuiApplication>

#include "NativeUiLaunchConfig.hpp"

namespace fo::gui {

int run_omni_shell(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const NativeUiLaunchConfig config = create_default_omni_ui_launch_config();
    if (!config.is_valid()) {
        return -1;
    }

    config.register_types();

    auto runtime = config.runtime_factory();
    if (!runtime) {
        return -1;
    }

    if (config.object_created_handler) {
        runtime->set_object_created_handler(config.object_created_handler);
    }

    runtime->load(config.main_qml);
    return app.exec();
}

} // namespace fo::gui
