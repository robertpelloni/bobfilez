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

QString selected_launch_profile_name()
{
    const QString configured_name = QProcessEnvironment::systemEnvironment().value(native_ui_profile_environment_variable()).trimmed();
    return configured_name.isEmpty() ? default_launch_profile_name() : configured_name;
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
    const QString requested_name = selected_launch_profile_name();
    NativeUiLaunchProfile profile = create_launch_profile_by_name(requested_name);

    if (profile.is_valid()) {
        return profile;
    }

    if (requested_name != default_launch_profile_name()) {
        qWarning() << "Unknown native UI launch profile requested via"
                   << native_ui_profile_environment_variable() << ":"
                   << requested_name << "- falling back to default profile";
        profile = create_launch_profile_by_name(default_launch_profile_name());
    }

    return profile;
}

} // namespace fo::gui
