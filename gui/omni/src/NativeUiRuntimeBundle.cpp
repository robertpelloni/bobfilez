#include "NativeUiRuntimeBundle.hpp"

#include <QString>

#include "NativeUiRuntime.hpp"
#include "OmniQmlRegistration.hpp"

namespace fo::gui {

bool NativeUiRuntimeBundle::is_valid() const
{
    return !name.isEmpty() && register_types && runtime_factory;
}

NativeUiRuntimeBundle create_default_omni_runtime_bundle()
{
    NativeUiRuntimeBundle bundle;
    bundle.name = QStringLiteral("qt-qml-default-runtime");
    bundle.register_types = []() {
        register_omni_qml_types();
    };
    bundle.runtime_factory = []() {
        return create_default_declarative_ui_runtime();
    };
    return bundle;
}

} // namespace fo::gui
