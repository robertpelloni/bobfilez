from __future__ import annotations

import re
import shutil
import sys
from pathlib import Path


def patch_import_prefix(content: str, build_dir: Path) -> str:
    pattern = re.compile(
        r'# Compute the installation prefix relative to this file\.\n'
        r'get_filename_component\(_IMPORT_PREFIX "\$\{CMAKE_CURRENT_LIST_FILE\}" PATH\)\n'
        r'get_filename_component\(_IMPORT_PREFIX "\$\{_IMPORT_PREFIX\}" PATH\)\n'
        r'get_filename_component\(_IMPORT_PREFIX "\$\{_IMPORT_PREFIX\}" PATH\)\n'
        r'if\(_IMPORT_PREFIX STREQUAL "/"\)\n'
        r'  set\(_IMPORT_PREFIX ""\)\n'
        r'endif\(\)'
    )
    replacement = (
        '# Compute the installation prefix relative to this file.\n'
        f'set(_IMPORT_PREFIX "{build_dir.as_posix()}")'
    )
    new_content, count = pattern.subn(replacement, content, count=1)
    if count != 1:
        raise RuntimeError('Could not patch _IMPORT_PREFIX block in export file')
    return new_content


def main() -> int:
    btk_root = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path('libs/btk').resolve()
    btk_build = btk_root / 'build-btk'
    if not (btk_build / 'BTKConfig.cmake').exists():
        print(f'[ERROR] BTK build tree not found at {btk_build}')
        return 1

    export_root = btk_build / 'CMakeFiles' / 'Export'
    export_dirs = sorted(
        [path for path in export_root.iterdir() if path.is_dir() and (path / 'CopperSpiceLibraryTargets.cmake').exists()]
    )
    if not export_dirs:
        print(f'[ERROR] Could not locate BTK export directory under {export_root}')
        return 1

    export_dir = export_dirs[-1]
    print(f'[INFO] Preparing BTK build-tree package surface from {export_dir}')

    files_to_copy = [
        'CopperSpiceLibraryTargets.cmake',
        'CopperSpiceLibraryTargets-release.cmake',
        'CopperSpiceBinaryTargets.cmake',
        'CopperSpiceBinaryTargets-release.cmake',
    ]
    for name in files_to_copy:
        shutil.copy2(export_dir / name, btk_build / name)

    macro_files = [
        'BTKMacros.cmake',
        'BTKDeploy.cmake',
        'CopperSpiceMacros.cmake',
        'CopperSpiceDeploy.cmake',
    ]
    for name in macro_files:
        shutil.copy2(btk_root / 'cmake' / name, btk_build / name)

    for name in ['CopperSpiceLibraryTargets.cmake', 'CopperSpiceBinaryTargets.cmake']:
        target_file = btk_build / name
        target_file.write_text(
            patch_import_prefix(target_file.read_text(encoding='utf-8'), btk_build),
            encoding='utf-8',
        )

    print(f'[INFO] BTK build-tree package files prepared in {btk_build}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
