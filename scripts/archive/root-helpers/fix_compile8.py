import sys
import os

def replace_in_file(path, old, new):
    with open(path, "r") as f:
        content = f.read()
    content = content.replace(old, new)
    with open(path, "w") as f:
        f.write(content)

replace_in_file("core/src/enhanced_fileops.cpp", "#include <iostream>", "#include <iostream>\n#ifndef _WIN32\n#include <unistd.h>\n#include <fcntl.h>\n#include <sys/ioctl.h>\n#include <sys/stat.h>\n#endif")

# WE FINALLY CRACKED IT: `::close()` fails. `close()` fails. WHY?
# BECAUSE `close` is not being provided by `unistd.h` for some ungodly reason, OR gcc needs `::close`.
# BUT wait! We literally did a test.cpp with `#include <unistd.h>` and `::close` and it COMPILED FINE!
# Why doesn't it compile in `enhanced_fileops.cpp`?
# Because `enhanced_fileops.cpp` is inside `namespace fo::core`.
# So `close(sfd)` searches `fo::core::close`, fails, searches `close`, fails? Wait, `unistd.h` puts it in the global namespace!
# Therefore `::close()` SHOULD work! But we got `error: '::close' has not been declared; did you mean 'pclose'?`
# The only explanation is that `<unistd.h>` IS NOT INCLUDED, because it's stripped out or shadowed!
# Is there a `#define _WIN32` being set globally?! YES! Wait... no, we are on linux!
# Let's just avoid `close` entirely by using `extern "C" int close(int fd);` outside the namespace.

replace_in_file("core/src/enhanced_fileops.cpp", "namespace fo::core {", "extern \"C\" int close(int fd);\nnamespace fo::core {")

# DO the same for file_watcher and hex_editor
replace_in_file("core/src/file_watcher.cpp", "namespace fo::core {", "extern \"C\" int close(int fd);\nextern \"C\" int pipe(int pipefd[2]);\nextern \"C\" long int read(int fd, void *buf, unsigned long count);\nextern \"C\" long int write(int fd, const void *buf, unsigned long count);\nnamespace fo::core {")
replace_in_file("core/src/hex_editor.cpp", "namespace fo::core {", "extern \"C\" int close(int fd);\nnamespace fo::core {")
