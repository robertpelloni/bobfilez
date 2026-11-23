> **SCOPE CUT — Nov 19, 2026: READ THIS FIRST**
>
> This document describes a **grand, aspirational architecture**, not the
> shipped product. As of v6.5.22 (Nov 17, 2026), bobfilez is a
> **file organizer**: a C++20 core (hashing / dedup, integrity scrub,
> similar-image search, video hashing, audio fingerprinting, OCR,
> batch rename, encrypted vault, hash-chained audit ledger) driven by
> a CLI and a web dashboard, with tests at lean ctest 28/28, JUCE-tree
> ctest 25/25, and dashboard wiring 18/18.
>
> The OmniShell "Windows 11 Replacement" shell, the 35+ Omni-tools,
> and the 39 QML panels below — including the panels whose "parity
> targets" name DaVinci Resolve, Ableton Live, Lightroom, IDA Pro,
> VeraCrypt, GitKraken, and other professional products — are
> **parked scaffolds, stubs, or plans, not built equivalents of those
> products**. Do not cite this document as evidence that they exist.
> The README is the source of truth for current status.

---

# ARCHITECTURE.md — Bobfilez v6.0.0 Grand Architecture

## Overview

Bobfilez is a C++20/Qt6 cross-platform file management suite that has evolved from a simple deduplication tool into a complete **Autonomous Operating System Shell** with 35+ integrated subsystems ("Omni-tools"), 160+ git submodules, and a unified scheduling backbone (Nexus Master Clock).

## System Diagram

```
┌─────────────────────────────── OmniShell (Windows 11 Replacement) ───────────────────────────────┐
│  Taskbar │ Start Menu │ Desktop Icons │ Window Manager │ Nexus Pulse │ Notification Center       │
├──────────┴────────────┴───────────────┴────────────────┴──────────────┴───────────────────────────┤
│                                                                                                   │
│  ┌── File Management ──┐  ┌── Creative Suite ──┐  ┌── AI & Intelligence ─┐  ┌── Security ──────┐ │
│  │ Explorer (Win11)     │  │ OmniVision (NLE)   │  │ OmniOracle (RAG LLM)│  │ OmniSec (r2/hc)  │ │
│  │ Enhanced Copy/Move   │  │ OmniAudio (DAW)    │  │ OmniClerk (AI Sec)  │  │ OmniCrypt (Stego)│ │
│  │ Batch Rename (15+)   │  │ Photo Develop (LR) │  │ Semantic Tagging     │  │ Encrypted Vault   │ │
│  │ Batch Convert (10+)  │  │ Photo Library (AP) │  │ Neural Bridge        │  │ PII Sentinel      │ │
│  │ Search (Everything)  │  │ Media Asset Mgr    │  │ OmniGraph (KG)       │  │ Digital Notary    │ │
│  │ Hex Editor (HxD)     │  │ Markdown Viewer    │  │ Document Embedder    │  │ Forensic Audit    │ │
│  │ Image Viewer (XnView)│  │ OmniPeek (QuickLk) │  │ CLIP Search          │  │ Self-Healing FS   │ │
│  └──────────────────────┘  └────────────────────┘  └──────────────────────┘  └──────────────────┘ │
│                                                                                                   │
│  ┌── Automation ────────┐  ┌── Infrastructure ──┐  ┌── Visualization ────┐  ┌── Dev & Sharing ─┐ │
│  │ OmniFlow (Blueprints)│  │ OmniMount (FUSE)   │  │ OmniVerse (3D)      │  │ OmniGit (libgit2)│ │
│  │ Shadow Sorter        │  │ OmniCluster (Grid) │  │ Data Topology (TM)  │  │ OmniTerminal (AI)│ │
│  │ Auto-Hierarchy (AI)  │  │ Data Swarm (P2P)   │  │ Visual Dedup (Swipe)│  │ OmniShare (HTTPS)│ │
│  │ Data Pruner (Rot)    │  │ Cloud (S3/GD/Az)   │  │ OmniGraph Viz       │  │ Web Dashboard    │ │
│  │ File Watcher (RT)    │  │ Remote (SFTP/SMB)  │  │                      │  │ WASM Bridge      │ │
│  │ OmniClerk Pipeline   │  │ VFS / Live Folders │  │                      │  │                   │ │
│  └──────────────────────┘  └────────────────────┘  └──────────────────────┘  └──────────────────┘ │
│                                                                                                   │
│  ┌── Time & Recovery ───┐  ┌── Fun & UX ────────┐                                                │
│  │ OmniTimeMachine (CDP)│  │ Gamification (XP)   │                                                │
│  │ Data Recovery / Heal │  │ Achievements        │                                                │
│  │ Zero-Copy (Reflink)  │  │ Daily Quests        │                                                │
│  └──────────────────────┘  └────────────────────┘                                                │
├───────────────────────────────────────────────────────────────────────────────────────────────────┤
│                              Nexus Master Clock (Priority Scheduler)                              │
│                              SQLite Database (6 Migrations)                                        │
│                              BobUI / OmniUI Toolkit (Qt6 + JUCE + ImGui)                         │
└───────────────────────────────────────────────────────────────────────────────────────────────────┘
```

## Core C++ Interface Count

| Category | Interfaces (.hpp) | Implementations (.cpp) |
|----------|-------------------|----------------------|
| File Management | 12 | 12 |
| AI / Neural | 8 | 8 |
| Security & Crypto | 6 | 6 |
| Media & Creative | 4 | 4 |
| Infrastructure | 8 | 8 |
| Visualization | 3 | 3 |
| Automation | 5 | 5 |
| Dev Tools | 4 | 4 |
| **Total** | **~50** | **~50** |

## QML Panel Count

| Panel | Parity Target |
|-------|---------------|
| ExplorerWindow | Windows 11 Explorer |
| SearchPanel | Everything + AgentRansack + grepWin |
| BatchRenamePanel | Bulk Rename Utility |
| BatchConvertPanel | HandBrake + XnConvert |
| EnhancedFileOpsPanel | TeraCopy + FastCopy + SuperCopier |
| HexEditorPanel | HxD + 010 Editor |
| ImageViewerPanel | XnViewMP |
| MarkdownViewerPanel | Typora + Obsidian |
| PhotoDevelopPanel | Adobe Lightroom Classic |
| PhotoLibraryPanel | Apple Photos + Google Photos |
| OmniVisionPanel | DaVinci Resolve (NLE) |
| OmniAudioPanel | Ableton Live (DAW) |
| OmniFlowPanel | Apple Shortcuts + Unreal Blueprints |
| OmniGraphPanel | Obsidian Graph View |
| OmniSecPanel | IDA Pro + Hashcat |
| OmniCryptPanel | VeraCrypt + OpenStego |
| OmniGitPanel | GitKraken + Sourcetree |
| OmniTerminalPanel | Windows Terminal + Copilot CLI |
| OmniSharePanel | WeTransfer (Self-Hosted) |
| OmniClusterPanel | BOINC + Folding@Home |
| OmniVersePanel | Spatial Computing |
| OmniClerkPanel | Autonomous AI Secretary |
| OmniMountPanel | Dokany/FUSE Manager |
| OraclePanel | ChatGPT (Local RAG) |
| TimeMachinePanel | macOS Time Machine |
| RecoveryPanel | Data Recovery Suite |
| ForensicPanel | Forensic Audit Ledger |
| NotaryPanel | Digital Signature Suite |
| VaultPanel | VeraCrypt Vault |
| TopologyPanel | WinDirStat Treemap |
| VisualDedupPanel | Tinder for Files |
| DigitalRotPanel | System Maintenance |
| GamificationPanel | XP/Achievement System |
| CloudPanel | S3/GDrive/Azure Manager |
| NetworkPanel | SFTP/SMB Manager |
| SwarmPanel | P2P Mesh Network |
| FileWatcherPanel | Real-Time Monitor |
| HierarchyPanel | AI Folder Generator |
| AssetManagerPanel | Media Asset Manager |
| **Total** | **39 QML Panels** |

## Technology Stack

- **Language**: C++20 (core), QML/JavaScript (UI), Java 21 (port), HTML/CSS/JS (web)
- **Build**: CMake + Ninja + vcpkg
- **UI Toolkit**: BTK — BTK/CopperSpice-line native framework, with the older BobUI/OmniUI path now being retired from active integration
- **Database**: SQLite3 (6 schema migrations, WAL mode)
- **AI Models**: ONNX Runtime (CLIP ViT-B/32, MiniLM-L6, MobileNet-v2, Tesseract v5)
- **Submodules**: 160+ (FFmpeg, ImageMagick, libgit2, radare2, hashcat, opencv, raylib, libsodium, etc.)

## Version History Summary

| Version | Codename | Key Feature |
|---------|----------|-------------|
| 2.0-2.4 | Foundation | CLI, Dedup, Cloud, Native UI groundwork |
| 2.5-2.8 | Media Analysis | Video Hash, Audio FP, Conversion, Rename, Search |
| 2.9-3.0 | Shell Transformation | OmniShell, Nexus, Zero-Copy, PII Sentinel |
| 3.1-3.5 | Intelligence | VFS, Gamification, Cloud Mount, Photo Suite, Native UI integration |
| 3.6-3.9 | Automation | Shadow Sorter, Remote Storage, Semantic Tagging, MAM |
| 4.0-4.2 | Distribution | Data Swarm, Window Tiling, Self-Healing, Digital Notary |
| 4.3-4.6 | Knowledge | OmniGraph, OmniFlow, TimeMachine, OmniOracle (RAG) |
| 4.7-4.9 | Production | OmniMount, OmniVision (NLE), OmniAudio (DAW) |
| 5.0-5.2 | Security | OmniSec, OmniClerk, OmniPeek |
| 5.3-5.5 | Advanced | OmniGit, OmniVerse (3D), OmniCrypt (Steganography) |
| 5.6-5.8 | Infrastructure | OmniCluster, OmniTerminal, OmniShare |
| **6.0** | **Grand Architecture** | **Documentation & Unification** |
