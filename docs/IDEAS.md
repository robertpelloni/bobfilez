# Ideas: Future Refactoring & Pivots

## Aggressive Refactoring
- **Move to Coroutines**: Replace the manual background thread spawning in `FileModel` and `IFileWatcher` with C++20 coroutines for cleaner asynchronous flow.
- **Kernel-Level Swarm**: Implement the Swarm discovery logic as a Linux kernel module (or Windows driver) for near-zero-latency peer detection.
- **WASM Search Engine**: Port the core `SearchEngine` to WebAssembly to allow identical performance in the `bobui_web` React frontend as the native CLI.

## Visionary Pivots
- **Autonomous Git**: Integrate the AutoDev protocol directly with `libgit2` to automatically perform commits and branches based on detected successful test cycles.
- **Data-Swarm Cloud**: Pivot from a peer-to-peer sync tool to a decentralized cloud storage provider where users trade local storage for encrypted redundancy across the swarm.
- **AI Secretary (Supervised)**: Train a local LLM on the human feedback received via the Protocol Intervention UI to eventually fully automate file filing without human confirmation.
