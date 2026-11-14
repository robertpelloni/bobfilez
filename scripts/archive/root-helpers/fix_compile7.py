import sys
import os

with open("core/src/enhanced_fileops.cpp", "r") as f:
    text = f.read()

# Since `close` isn't declared, maybe it requires `#include <unistd.h>` to be placed inside the `fo::core` namespace? No, it's just missing entirely. Let's do it right before `try_zero_copy`.
text = text.replace('static bool try_zero_copy', '#ifndef _WIN32\n#include <unistd.h>\n#endif\nstatic bool try_zero_copy')

with open("core/src/enhanced_fileops.cpp", "w") as f:
    f.write(text)

with open("core/src/file_watcher.cpp", "r") as f:
    text = f.read()
text = text.replace('namespace fo::core {', '#ifndef _WIN32\n#include <unistd.h>\n#endif\nnamespace fo::core {')
with open("core/src/file_watcher.cpp", "w") as f:
    f.write(text)

with open("core/src/hex_editor.cpp", "r") as f:
    text = f.read()
text = text.replace('namespace fo::core {', '#ifndef _WIN32\n#include <unistd.h>\n#endif\nnamespace fo::core {')
with open("core/src/hex_editor.cpp", "w") as f:
    f.write(text)
