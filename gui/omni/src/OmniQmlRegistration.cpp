#include "OmniQmlRegistration.hpp"

#include <qqml.h>
#include <OmniQmlRegistration.h>

#include "FileModel.h"
#include "TreemapModel.h"
#include "NativeMarkdownView.h"

namespace fo::gui {

void register_omni_qml_types()
{
    OmniUI::registerQmlTypes();
    qmlRegisterType<FileModel>("Omni.File", 1, 0, "FileModel");
    qmlRegisterType<fo::gui::TreemapModel>("Omni.Viz", 1, 0, "TreemapModel");
    qmlRegisterType<fo::gui::NativeMarkdownView>("Omni.Native", 1, 0, "MarkdownView");
}

} // namespace fo::gui
