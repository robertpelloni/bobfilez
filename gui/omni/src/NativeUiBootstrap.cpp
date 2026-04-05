#include "NativeUiBootstrap.hpp"

#include <QGuiApplication>

#include "NativeUiProfileRegistry.hpp"

namespace fo::gui {

int run_omni_shell(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const NativeUiLaunchProfile profile = create_launch_profile_from_selection(argc, argv);
    if (!profile.is_valid()) {
        return -1;
    }

    profile.runtime_bundle.register_types();

    auto runtime = profile.runtime_bundle.runtime_factory();
    if (!runtime) {
        return -1;
    }

    if (profile.object_created_handler) {
        runtime->set_object_created_handler(profile.object_created_handler);
    }

    runtime->load(profile.main_qml);
    return app.exec();
}

} // namespace fo::gui
