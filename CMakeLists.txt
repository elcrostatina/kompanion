# SPDX-FileCopyrightText: 2026 Kompanion contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

project(Kompanion VERSION 0.1.0 LANGUAGES CXX)

set(QT_MIN_VERSION 6.6.0)
set(KF_MIN_VERSION 6.8.0)
set(KOMPANION_APP_ID io.github.elcrostatina.Kompanion)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(KOMPANION_BUILD_TESTS "Build the unit tests" ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core Gui Widgets Qml Quick QuickControls2 Network Sql WebEngineQuick)

if(KOMPANION_BUILD_TESTS)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
endif()

qt_policy(SET QTP0001 NEW)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons Config I18n DBusAddons GlobalAccel Kirigami SyntaxHighlighting KIO StatusNotifierItem)

find_package(Qt6Keychain REQUIRED)

find_package(LayerShellQt REQUIRED)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(src)

if(KOMPANION_BUILD_TESTS)
    enable_testing()
    add_subdirectory(autotests)
endif()

install(FILES ${KOMPANION_APP_ID}.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ${KOMPANION_APP_ID}.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/icons/${KOMPANION_APP_ID}.svg
        DESTINATION ${KDE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
