# Explorer File Manager: Implementation Plan & Progress

## 1. Planned Functionality Categories

### Category A: Core Navigation & View (The "Windows 11" Experience)
- [x] **Tabbed Interface:** Support for multiple directory tabs.
- [x] **Breadcrumbs & Address Bar:** Clickable segments for navigation (editable text input).
- [x] **Sidebar Navigation:** Quick Access, OneDrive, This PC, Network (links working).
- [ ] **View Modes:** List (current), Details (expanded), Icons (Grid), Tiles.
- [x] **Context Menus:** Modern, right-click file/folder actions.
- [x] **Status Bar:** Item counts and selection info.

### Category B: Core Operations
- [ ] **File Lifecycle:** Create, Open, Rename, Delete.
- [ ] **Clipboard:** Copy, Paste, Move.
- [ ] **Drag & Drop:** Native DnD support between folders.
- [ ] **Properties Dialog:** Advanced file info.

### Category C: Advanced OmniEngine Features
- [x] **Deduplication:** UI trigger for finding duplicates in current folder.
- [x] **Duplicate UI Visuals:** "👯" icon and red text for duplicate files.
- [ ] **Smart Date Display:** Show "Fused Date" instead of just mtime.
- [ ] **AI Classification Icons:** Dynamic icons based on file content (OCR/Image Analysis).

### Category D: Shell Integration
- [x] **Taskbar:** Start Menu button, App pinning, Clock/Date.
- [x] **Start Menu:** Search, Pinned Apps.
- [x] **Dashboard:** Quick access to system settings and notifications.
- [x] **Desktop:** Icon support, wallpaper simulation.

### Category E: Integrated Image Management Suite (bobphotos)
- [ ] **Photo Library View:** Timeline-based viewing similar to Google/Apple Photos and Lightroom Classic.
- [ ] **AI Face Recognition:** Automatic identity grouping and tagging for people and pets.
- [ ] **Image Classification:** Smart categorization (landscapes, documents, receipts, etc.).
- [ ] **Smart Deduplication:** Analyze duplicate/similar images, recommend and keep the "best" version based on quality/metadata.
- [ ] **Auto-Rotate:** ML-driven automatic orientation correction.
- [ ] **EXIF Tools:** Advanced scrubber, privacy corrector, and mass-metadata editor.
- [ ] **Format Conversion & Extraction:** 
    - Transcode modern formats (e.g., HEIC to JXL, WebP to AVIF).
    - Extract stills from rapid-fire bursts, Live Photos, and animations.

### Category F: Advanced Dual-Pane Operations (Salamander 100% Parity)
- [ ] **Classic Dual-Pane Mode:** Seamless side-by-side file management.
- [ ] **Keyboard-First Workflow:** 100% navigability and operation via keyboard shortcuts (F5 Copy, F6 Move, etc.).
- [ ] **Advanced Viewers:** Integrated, instant hex, text, and media viewers.
- [ ] **Plugin Architecture:** Support for custom modules (FTP/SFTP, advanced archive handling for 7z/RAR, encryption).
- [ ] **Power User Tools:** Advanced batch renaming, file splitting/combining, and deep attribute manipulation.

## 2. Progress Summary (2026-04-01)
- **OmniUI Shell Skeleton:** Established in `main.qml`.
- **Explorer Window Enhancement:** Updated `explorerWindow` with:
    - Mica/Acrylic-style Title Bar with Tabs.
    - Toolbar with Address Bar and Search.
    - Command Bar with "Deduplicate" action.
    - Split-view Sidebar for navigation.
    - Improved File List with "Details" headers (Name, Date, Type, Size).
    - Status Bar showing item/selection counts.
- **FileModel Core:** Implementation of `FileModel.cpp/h` provides the bridge to `fo_core`.

## 3. Next Steps
1. **Functional Address Bar:** Make the address bar editable and update the model when the path changes.
2. **Details View Expansion:** Ensure columns like "Type" and "Date Modified" are correctly populated in the model.
3. **Context Menus:** Implement a QML `Menu` for right-click actions on files.
4. **Sidebar Navigation Logic:** Make the Sidebar items functional (e.g., clicking "This PC" opens `Computer`).
