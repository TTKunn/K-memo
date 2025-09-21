cmake_minimum_required(VERSION 3.16)

project(k-memo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        kmemo.cpp
        kmemo.h
        kmemo.ui

        # Models
        models/task.h
        models/task.cpp
        models/taskmodel.h
        models/taskmodel.cpp

        # Database
        database/databasemanager.h
        database/databasemanager.cpp

        # Managers
        managers/traymanager.h
        managers/traymanager.cpp
        managers/notificationmanager.h
        managers/notificationmanager.cpp

        # Icon Manager
        IconManager.h
        IconManager.cpp

        # Custom Widget Components
        TaskDetailWidget.h
        TaskDetailWidget.cpp
        TaskStatsWidget.h
        TaskStatsWidget.cpp
        QuickAddWidget.h
        QuickAddWidget.cpp
)

# Resource files
set(RESOURCE_FILES
    resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(k-memo
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_add_resources(k-memo "app_resources" FILES ${RESOURCE_FILES})
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET k-memo APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(k-memo SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(k-memo
            ${PROJECT_SOURCES}
            ${RESOURCE_FILES}
        )
    endif()
endif()

target_include_directories(k-memo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(k-memo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.k-memo)
endif()
set_target_properties(k-memo PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS k-memo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(k-memo)
endif()
