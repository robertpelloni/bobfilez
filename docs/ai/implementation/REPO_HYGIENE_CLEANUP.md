# Repo Hygiene Cleanup — 2026-04-03

## Goal
Reduce repository noise from generated build artifacts and verify whether cleanup improves day-to-day `git status` usability.

## Cleanup Performed
The following generated component-level CMake output trees were removed from the working tree:

- `benchmarks/build_output/`
- `cli/build_output/`
- `core/build_output/`
- `fuzz/build_output/`
- `gui/build_output/`
- `tests/build_output/`
- `tests/build/`

## Important Observation
Several `build_output/` trees were not merely untracked local artifacts — some were actually present in the git index as tracked files. In practice, these were CMake-generated snapshots such as:
- `CMakeCache.txt`
- `CMakeFiles/4.2.3/CMakeCCompiler.cmake`
- `CompilerIdC*`
- `CompilerIdCXX*`
- `CMakeConfigureLog.yaml`

These files are environment-specific build outputs and are poor candidates for source control.

## Result
The cleanup successfully removed the tracked/generated `build_output` snapshots from the working tree for the targeted component directories.

## Remaining Problem
Even after this cleanup, root-level `git status` still emitted filename-too-long warnings for paths of the form:

- `tests/test_cmake_build/subdirectory_embed/build_output/pybind11/...`
- `tests/test_cmake_build/subdirectory_function/build_output/pybind11/...`
- `tests/test_cmake_build/subdirectory_target/build_output/pybind11/...`

## Interpretation
This suggests the remaining warning source is **not fully explained by the tracked `build_output/` trees that were removed**.

Most likely explanations:
1. There are additional generated long-path trees not visible through normal short-path enumeration.
2. A nested pybind11/CMake test artifact tree exists in a form that Windows path handling and normal listing APIs do not expose cleanly.
3. Additional cleanup may require Windows extended-path tooling or a more targeted pruning script.

## Additional Diagnostic Discovery
A useful operational workaround was confirmed:

- `git status --untracked-files=no`

On this repo/host, the tracked-only status view avoids the long-path warning path and still reports the important day-to-day signals:
- tracked file modifications
- deletions
- dirty submodules

This means the remaining warning is specifically tied to **untracked-file scanning**, not the normal tracked-change path.

To standardize this safer view, the repo now includes:
- **`scripts/repo_status.py`**

That helper runs a tracked-only status and prints a note about when full `git status` may still be noisy.

## Recommended Follow-up
1. Use `python scripts/repo_status.py` for routine tracked-change inspection on affected Windows hosts.
2. Re-run full `git status` only when untracked-file visibility is specifically needed.
3. If the long-path warning still needs full resolution, perform a dedicated extended-path cleanup pass for `tests/test_cmake_build/`.
4. Keep generated build artifacts out of version control going forward via `.gitignore` plus periodic cleanup.
