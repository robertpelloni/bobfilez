#pragma once

#include <QUrl>
#include <QString>

#include "NativeUiRuntimeBundle.hpp"

namespace fo::gui {

struct NativeUiLaunchProfile {
    QString name;
    QUrl main_qml;
    NativeUiRuntimeBundle runtime_bundle;
    ObjectCreatedHandler object_created_handler;

    bool is_valid() const;
};

NativeUiLaunchProfile create_default_omni_launch_profile();
NativeUiLaunchProfile create_dashboard_only_launch_profile();

} // namespace fo::gui
