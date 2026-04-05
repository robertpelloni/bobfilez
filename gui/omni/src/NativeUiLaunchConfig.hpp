#pragma once

#include <QUrl>

#include <functional>

#include "NativeUiRuntime.hpp"

namespace fo::gui {

using QmlRegistrationRoutine = std::function<void()>;
using DeclarativeUiRuntimeFactory = std::function<std::unique_ptr<DeclarativeUiRuntime>()>;

struct NativeUiLaunchConfig {
    QUrl main_qml;
    QmlRegistrationRoutine register_types;
    DeclarativeUiRuntimeFactory runtime_factory;
    ObjectCreatedHandler object_created_handler;

    bool is_valid() const;
};

NativeUiLaunchConfig create_default_omni_ui_launch_config();

} // namespace fo::gui
