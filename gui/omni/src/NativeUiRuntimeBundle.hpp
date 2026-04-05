#pragma once

#include <functional>
#include <memory>

#include <QString>

#include "NativeUiRuntime.hpp"

namespace fo::gui {

using QmlRegistrationRoutine = std::function<void()>;
using DeclarativeUiRuntimeFactory = std::function<std::unique_ptr<DeclarativeUiRuntime>()>;

struct NativeUiRuntimeBundle {
    QString name;
    QmlRegistrationRoutine register_types;
    DeclarativeUiRuntimeFactory runtime_factory;

    bool is_valid() const;
};

NativeUiRuntimeBundle create_default_omni_runtime_bundle();

} // namespace fo::gui
