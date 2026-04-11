# BTK Rename Buckets

## Purpose
Classify the current `CopperSpice` / `Cs*` naming surface into migration buckets so the BTK rename proceeds with minimal breakage.

## Bucket A - Safe Alias First
These are high-value and relatively low-risk to alias immediately:
- CMake package names (`BTK` alongside `CopperSpice`)
- imported library targets (`BTK::Core`, `BTK::Gui`, `BTK::Network`, etc.)
- helper macros/functions (`BTK_RESOURCES`, `btk_copy_library`, `btk_copy_plugins`)
- project metadata and top-level docs

### Status
Implemented in this session.

## Bucket B - Moderate Risk, Good Next Candidates
These are public but can likely be migrated with compatibility aliases or wrappers:
- selected public top-level class/type aliases where names are mostly branding wrappers rather than deep ABI anchors
- include-path convenience wrappers for BTK-branded umbrella headers
- developer-facing examples and downstream build templates

### Status
A first wave of this bucket is now implemented through:
- `<QtCore/BTKCore>`
- `<QtCore/BTKString>`
- `<QtCore/BTKPointer>`
- `<QtCore/BTKSignal>`

### Recommended approach
- add tiny, targeted `using` aliases or forwarding headers
- do not mass-rename headers yet
- prefer opt-in BTK names that coexist with legacy names

## Bucket C - ABI-Sensitive Public Types
These need careful staged migration:
- `CsString::*` types
- `CsPointer::*` types
- `CsSignal::*` types
- library names embedded across generated package exports and install layouts
- public macros consumed by downstream code

### Why risky
These names are deeply embedded in headers, templates, namespaces, and downstream user code. Abrupt replacement would likely create source and ABI churn.

## Bucket D - File-System / Layout Renames
These should happen late:
- `cs_*.h/.cpp` files
- install path names
- package directory layout
- generated header names like `cs_build_info.h`

### Why late
These are mechanically noisy, easy to get wrong, and do not provide enough immediate user value compared with alias-based migration.

## Bucket E - Strategic Non-Rename Work That Must Precede Big Renames
These are not naming tasks but are necessary before larger public churn:
- define modern declarative/runtime direction
- define multi-user ownership/focus primitives
- decide which legacy subsystems remain first-class and which are replaced
- validate BTK package consumption from downstream examples

## Recommendation
Continue the BTK rename as a compatibility-first program:
1. aliases and wrappers
2. targeted public API aliases
3. downstream validation
4. install/layout migration
5. eventual deprecation of legacy names
