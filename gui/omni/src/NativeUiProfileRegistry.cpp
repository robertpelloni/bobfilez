#include "NativeUiProfileRegistry.hpp"

#include <QProcessEnvironment>
#include <QDebug>

namespace fo::gui {

QString default_runtime_bundle_name()
{
    return QStringLiteral("qt-qml-default-runtime");
}

QString default_launch_profile_name()
{
    return QStringLiteral("omni-shell-default");
}

QString native_ui_profile_environment_variable()
{
    return QStringLiteral("BOBFILEZ_NATIVE_UI_PROFILE");
}

QString native_ui_profile_argument_prefix()
{
    return QStringLiteral("--native-ui-profile=");
}

QString selected_launch_profile_name()
{
    const QString configured_name = QProcessEnvironment::systemEnvironment().value(native_ui_profile_environment_variable()).trimmed();
    return configured_name.isEmpty() ? default_launch_profile_name() : configured_name;
}

QString selected_launch_profile_name(int argc, char *argv[])
{
    const QString argument_prefix = native_ui_profile_argument_prefix();

    for (int i = 1; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        if (argument.startsWith(argument_prefix)) {
            const QString requested_name = argument.mid(argument_prefix.size()).trimmed();
            if (!requested_name.isEmpty()) {
                return requested_name;
            }
        }
    }

    return selected_launch_profile_name();
}

QStringList available_runtime_bundle_names()
{
    return {default_runtime_bundle_name()};
}

QStringList available_launch_profile_names()
{
    return {default_launch_profile_name()};
}

NativeUiRuntimeBundle create_runtime_bundle_by_name(const QString &name)
{
    if (name == default_runtime_bundle_name()) {
        return create_default_omni_runtime_bundle();
    }

    return {};
}

NativeUiLaunchProfile create_launch_profile_by_name(const QString &name)
{
    if (name == default_launch_profile_name()) {
        return create_default_omni_launch_profile();
    }

    return {};
}

NativeUiLaunchProfile create_launch_profile_from_environment()
{
    return create_launch_profile_from_selection(0, nullptr);
}

NativeUiLaunchProfile create_launch_profile_from_selection(int argc, char *argv[])
{
    const QString requested_name = (argc > 0 && argv != nullptr)
        ? selected_launch_profile_name(argc, argv)
        : selected_launch_profile_name();

    NativeUiLaunchProfile profile = create_launch_profile_by_name(requested_name);

    if (profile.is_valid()) {
        return profile;
    }

    if (requested_name != default_launch_profile_name()) {
        qWarning() << "Unknown native UI launch profile requested via"
                   << native_ui_profile_environment_variable() << "or"
                   << native_ui_profile_argument_prefix() << ":"
                   << requested_name << "- falling back to default profile";
        profile = create_launch_profile_by_name(default_launch_profile_name());
    }

    return profile;
}

} // namespace fo::gui
