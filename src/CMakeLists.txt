# SPDX-FileCopyrightText: 2026 Kompanion contributors
# SPDX-License-Identifier: GPL-3.0-or-later

add_subdirectory(core)

qt_add_executable(kompanion
    main.cpp
    app/PanelController.cpp
    app/PanelDocking.cpp
    app/LayerShellSetup.cpp
    app/FramelessDocking.cpp
    app/ShortcutManager.cpp
    app/ProviderModel.cpp
    app/ProviderSelection.cpp
    app/OverrideReloader.cpp
    app/WebProviderProfile.cpp
    app/WebProfileRegistry.cpp
    app/KeychainSecretStore.cpp
    app/ChatListModel.cpp
    app/MessageModel.cpp
    app/ChatController.cpp
    app/ChatControllerRegistry.cpp
    app/CodeBlockHighlighter.cpp
    app/FaviconStore.cpp
    app/ProviderEditor.cpp
    app/PreferencesController.cpp
    app/TrayIcon.cpp
)

qt_add_qml_module(kompanion
    URI io.github.elcrostatina.kompanion
    VERSION 1.0
    NO_IMPORT_SCAN
    QML_FILES
        qml/Main.qml
        qml/ProvidersPage.qml
        qml/ProviderRail.qml
        qml/ProviderDisc.qml
        qml/RailItem.qml
        qml/RailButton.qml
        qml/ProviderStack.qml
        qml/WebProviderView.qml
        qml/LocalChatView.qml
        qml/ChatListPage.qml
        qml/ConversationPage.qml
        qml/MessageDelegate.qml
        qml/CodeBlock.qml
        qml/Composer.qml
        qml/SettingsPage.qml
        qml/ProviderSettingsCard.qml
        qml/AddProviderDialog.qml
        qml/SwitchRow.qml
)

target_include_directories(kompanion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/app)

target_link_libraries(kompanion PRIVATE
    kompanion_core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::WebEngineQuick
    Qt6::Sql
    Qt6Keychain::Qt6Keychain
    KF6::SyntaxHighlighting
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::I18nQml
    KF6::DBusAddons
    KF6::KIOGui
    KF6::StatusNotifierItem
    KF6::GlobalAccel
    LayerShellQt::Interface
)

install(TARGETS kompanion ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
