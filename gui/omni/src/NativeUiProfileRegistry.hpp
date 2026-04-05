#pragma once

#include <QString>
#include <QStringList>

#include "NativeUiLaunchProfile.hpp"
#include "NativeUiRuntimeBundle.hpp"

namespace fo::gui {

QString default_runtime_bundle_name();
QString default_launch_profile_name();

QStringList available_runtime_bundle_names();
QStringList available_launch_profile_names();

NativeUiRuntimeBundle create_runtime_bundle_by_name(const QString &name);
NativeUiLaunchProfile create_launch_profile_by_name(const QString &name);

} // namespace fo::gui
