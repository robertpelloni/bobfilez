#include "NativeUiLaunchProfile.hpp"

#include <QCoreApplication>
#include <QString>

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

bool NativeUiLaunchProfile::is_valid() const
{
    return !name.isEmpty() && !main_qml.isEmpty() && runtime_bundle.is_valid();
}

NativeUiLaunchProfile create_default_omni_launch_profile()
{
    NativeUiLaunchProfile profile;
    profile.name = QStringLiteral("omni-shell-default");
    profile.main_qml = QUrl(QStringLiteral("qrc:/main.qml"));
    profile.runtime_bundle = create_default_omni_runtime_bundle();
    profile.object_created_handler = create_root_object_failure_handler(profile.main_qml);
    return profile;
}

NativeUiLaunchProfile create_dashboard_only_launch_profile()
{
    NativeUiLaunchProfile profile;
    profile.name = QStringLiteral("omni-dashboard-only");
    profile.main_qml = QUrl(QStringLiteral("qrc:/DashboardShell.qml"));
    profile.runtime_bundle = create_default_omni_runtime_bundle();
    profile.object_created_handler = create_root_object_failure_handler(profile.main_qml);
    return profile;
}

NativeUiLaunchProfile create_explorer_only_launch_profile()
{
    NativeUiLaunchProfile profile;
    profile.name = QStringLiteral("omni-explorer-only");
    profile.main_qml = QUrl(QStringLiteral("qrc:/ExplorerShell.qml"));
    profile.runtime_bundle = create_default_omni_runtime_bundle();
    profile.object_created_handler = create_root_object_failure_handler(profile.main_qml);
    return profile;
}

} // namespace fo::gui
