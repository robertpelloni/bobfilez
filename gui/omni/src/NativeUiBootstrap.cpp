#include "NativeUiBootstrap.hpp"

#include <QGuiApplication>
#include <QString>
#include <QTextStream>

#include "NativeUiProfileRegistry.hpp"

namespace fo::gui {
namespace {

void print_available_launch_profiles()
{
    QTextStream out(stdout);
    out << "Available native UI launch profiles:\n";
    for (const auto &name : available_launch_profile_names()) {
        const QString marker = (name == default_launch_profile_name()) ? QStringLiteral(" (default)") : QString();
        out << "  - " << name << marker << "\n";
    }
    out << "Selection order: --native-ui-profile=<name> > "
        << native_ui_profile_environment_variable() << " > default\n";
}

} // namespace

int run_omni_shell(int argc, char *argv[])
{
    if (should_list_launch_profiles(argc, argv)) {
        print_available_launch_profiles();
        return 0;
    }

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
