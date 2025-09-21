# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

K-memo is a desktop task management and note-taking application built with Qt/C++. It supports comprehensive task management with categories, tags, priorities, and notifications. Uses CMake as the build system and follows MVC architecture patterns.

### Current Development Status
- **Phase**: Execution Stage - Basic Architecture Setup
- **Progress**: Stage 1 Task 1.1 Completed ✅
- **Next Task**: Task 1.2 - Data Model Design & Implementation
- **Development Approach**: RIPER-5 Workflow (Research → Investigate → Plan → Execute → Review)
- **Selected Solution**: Solution 2 (Balanced Approach) - SQLite + QSystemTrayIcon + MVC Architecture

### Key Requirements
- **Core Features**: Task CRUD operations, categories, tags, priority levels, Windows 11 notifications, system tray integration
- **Removed Features**: Search/filter functionality, theme customization (per user request)
- **Storage**: Local SQLite database (Phase 1), future cloud integration planned (Phase 2)
- **Target Platform**: Windows 11 desktop application

## Architecture

### Core Components
- **Main Application**: `k-memo/main.cpp` - Entry point that initializes QApplication and shows the main window
- **Main Window**: `k-memo/kmemo.h/cpp` - Core application window class inheriting from QMainWindow
- **UI Definition**: `k-memo/kmemo.ui` - Qt Designer UI file defining the interface layout
- **Build System**: `k-memo/CMakeLists.txt` - CMake configuration supporting Qt5/Qt6

### Data Layer
- **Task Model**: `k-memo/models/task.h/cpp` - Core task entity with priority, status, categories, and tags
- **Task Model Controller**: `k-memo/models/taskmodel.h/cpp` - QAbstractListModel for task list management
- **Database Manager**: `k-memo/database/databasemanager.h/cpp` - Singleton SQLite database interface with CRUD operations

### System Integration
- **Tray Manager**: `k-memo/managers/traymanager.h/cpp` - System tray integration with task count display
- **Notification Manager**: `k-memo/managers/notificationmanager.h/cpp` - Windows notification system integration

The application uses Qt's Model-View architecture with automatic UI compilation (AUTOUIC), meta-object compilation (AUTOMOC), and resource compilation (AUTORCC). Database operations are handled through a singleton pattern with signal-based updates.

### Technical Architecture Details

#### Data Flow Architecture
```
UI Layer (kmemo.cpp/ui)
    ↕ (Qt Signals/Slots)
Model Layer (TaskModel)
    ↕ (Database Signals)
Database Layer (DatabaseManager - Singleton)
    ↕ (SQLite Operations)
SQLite Database (Local Storage)
```

#### Key Design Patterns
- **Singleton Pattern**: DatabaseManager ensures single database connection
- **Observer Pattern**: Qt signals/slots for component communication
- **Model-View Pattern**: TaskModel provides data abstraction for UI
- **Factory Pattern**: Task creation with UUID generation

#### Component Dependencies
- **Main Window** → TaskModel → DatabaseManager → SQLite
- **TrayManager** → Main Window (show/hide control)
- **NotificationManager** → TrayManager (notification display)
- **TaskModel** ↔ DatabaseManager (bidirectional data sync)

## Build Commands

### Building the Application (Windows PowerShell)
```powershell
# Navigate to k-memo directory
cd k-memo

# Create build directory (if not exists)
if (-not (Test-Path build)) { mkdir build }
cd build

# Configure with CMake (uses Visual Studio 2022 by default)
cmake ..

# Build the application (Debug configuration)
cmake --build . --config Debug

# Run the application
.\Debug\k-memo.exe
```

### Alternative Build Commands (Cross-platform)
```bash
# Configure with specific build type
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build with specific configuration
cmake --build . --config Debug
```

### Development Build Commands
```bash
# Clean build
cmake --build . --target clean

# Install to system (optional)
cmake --build . --target install
```

### Qt Creator Integration
Open `k-memo/CMakeLists.txt` in Qt Creator and use the IDE's integrated build system. Qt Creator automatically handles the CMake configuration and provides debugging support.

## Project Structure

```
k-memo/
├── CMakeLists.txt              # Build configuration with Qt6/Qt5 support
├── main.cpp                    # Application entry point
├── kmemo.h/.cpp/.ui           # Main window implementation
├── models/                     # Data models
│   ├── task.h/.cpp            # Task entity class with enums and JSON serialization
│   └── taskmodel.h/.cpp       # QAbstractListModel for task management
├── database/                   # Data persistence layer
│   └── databasemanager.h/.cpp # Singleton SQLite database manager
├── managers/                   # System integration managers
│   ├── traymanager.h/.cpp     # System tray functionality
│   └── notificationmanager.h/.cpp # Windows notification system
└── build/                     # Build output directory
```

## Key Dependencies

The application requires Qt6 (or Qt5) with the following modules:
- **Qt::Widgets** - UI framework
- **Qt::Sql** - SQLite database integration

## Development Notes

- **Qt Version**: Supports both Qt5 and Qt6 (automatically detected via CMake)
- **C++ Standard**: C++17
- **Database**: SQLite with schema versioning and migration support
- **Architecture Pattern**: MVC with singleton database manager
- **Target Platform**: Primarily Windows 11 (with cross-platform Qt support)
- **UI Framework**: Qt Widgets with Designer-based UI files

## Data Model Architecture

### Task Entity
- Supports priorities (Low, Normal, High, Urgent) and status tracking (Pending, InProgress, Completed, Cancelled)
- Includes categories, tags, reminders, and JSON serialization
- UUID-based identification system

### Database Schema
- **tasks** table: Core task data with timestamps and metadata
- **task_tags** table: Many-to-many relationship for tag associations
- Version-controlled schema with migration support

## Implementation Status & Next Steps

### Completed Components ✅
- **Project Structure**: Complete MVC directory organization
- **CMake Configuration**: Qt6::Widgets + Qt6::Sql modules configured
- **Core Class Headers**: All primary class interfaces defined
- **Build Environment**: Successfully compiles and runs in VSCode + Visual Studio 2022

### Current Implementation Status
- **Task Entity**: Header complete, basic implementation with stubs
- **DatabaseManager**: Header complete, basic SQLite setup with stubs
- **TaskModel**: Header complete, QAbstractListModel framework with stubs
- **TrayManager**: Header complete, system tray integration framework
- **NotificationManager**: Header complete, notification system framework

### Next Development Tasks
1. **Task 1.2**: Complete Task class implementation (JSON serialization, validation)
2. **Task 1.3**: Complete DatabaseManager implementation (CRUD operations, schema)
3. **Task 2.1**: Integrate TaskModel with DatabaseManager
4. **Task 2.2**: Design and implement main UI interface

### Development Guidelines
- Follow RIPER-5 workflow methodology
- Update progress in `project_document/[002]K-memo方案二详细实施计划.md`
- All stub methods need full implementation
- Maintain MVC architecture separation
- Use Qt's signal-slot mechanism for component communication

## Generated Files

The following files are auto-generated and should not be edited manually:
- `ui_kmemo.h` - Generated from `kmemo.ui`
- `moc_*.cpp/h` - Meta-object compiler output for Qt signal/slot system
- Build artifacts in `build/` directory

## Project Documentation

### Planning Documents
- `project_document/[001]K-memo桌面便签应用开发计划.md` - Overall project plan
- `project_document/[002]K-memo方案二详细实施计划.md` - Detailed implementation plan with progress tracking
- `CLAUDE.md` - This development guidance document

### Development Environment
- **IDE**: VSCode (primary) + Qt Creator (alternative)
- **Compiler**: Visual Studio 2022 (MSVC 19.44.35213.0)
- **Build System**: CMake 3.30.5
- **Qt Version**: Qt6 (auto-detected, Qt5 fallback supported)
- **Database**: SQLite (via Qt6::Sql module)

## Important Development Notes

### Code Implementation Status
⚠️ **Critical**: Many methods in the current codebase are **stub implementations** that return default values or false. These need to be fully implemented:

#### DatabaseManager Stubs (Need Implementation)
- `updateTask()`, `deleteTask()`, `getTask()`
- `getTasksByCategory()`, `getTasksByStatus()`, `getTasksByPriority()`
- `getOverdueTasks()`, `getTodayTasks()`
- `removeTagFromTask()`, `getAllTags()`, `getAllCategories()`
- Statistics methods, configuration methods, backup/restore methods

#### TaskModel Stubs (Need Implementation)
- `updateTask()`, `removeTask()`, `getTask(QString)`
- `setFilter()`, `clearFilter()`, `setSortOrder()`, `setSortRole()`
- `getCompletedCount()`, `getPendingCount()`

### Development Priorities
1. **Complete Core CRUD Operations** - Essential for basic functionality
2. **Implement Database Schema** - Tables creation and data persistence
3. **Connect UI to Data Model** - Make the interface functional
4. **Add System Integration** - Tray and notifications

### Testing Strategy
- Build and run application after each major component completion
- Test database operations with sample data
- Verify UI responsiveness and data binding
- Test system tray and notification functionality on Windows 11

### Common Issues & Solutions
- **Include Path Issues**: Ensure `target_include_directories(k-memo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})` is set
- **Qt Module Missing**: Verify both `find_package` and `target_link_libraries` include required Qt modules
- **Stub Methods**: Replace all stub implementations with actual functionality before testing features