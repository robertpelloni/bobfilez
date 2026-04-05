#include "NativeUiProfileRegistry.hpp"

namespace fo::gui {

QString default_runtime_bundle_name()
{
    return QStringLiteral("qt-qml-default-runtime");
}

QString default_launch_profile_name()
{
    return QStringLiteral("omni-shell-default");
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

} // namespace fo::gui
