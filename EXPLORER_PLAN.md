# Explorer File Manager: Implementation Plan & Progress

## 1. Planned Functionality Categories

### Category A: Core Navigation & View (The "Windows 11" Experience)
- [x] **Tabbed Interface:** Support for multiple directory tabs.
- [ ] **Breadcrumbs & Address Bar:** Clickable segments for navigation.
- [x] **Sidebar Navigation:** Quick Access, OneDrive, This PC, Network.
- [ ] **View Modes:** List (current), Details (expanded), Icons (Grid), Tiles.
- [ ] **Context Menus:** Modern, right-click file/folder actions.
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
