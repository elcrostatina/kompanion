# SPDX-FileCopyrightText: 2026 Kompanion contributors
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(kompanion_core STATIC
    ApiUrl.cpp
    AppPaths.cpp
    BaseStyle.cpp
    Autostart.cpp
    AppSettings.cpp
    ChatExporter.cpp
    ChatStore.cpp
    EmptyFile.cpp
    FileNaming.cpp
    InMemorySecretStore.cpp
    MarkdownBlocks.cpp
    NavigationPolicy.cpp
    OpenAiClient.cpp
    OpenAiProtocol.cpp
    OverrideFiles.cpp
    OverrideScripts.cpp
    PageColorScript.cpp
    ProfileStorage.cpp
    ProviderPresets.cpp
    ProviderRegistry.cpp
    SecretStore.cpp
    SiteDataScripts.cpp
    SseParser.cpp
    TitleGenerator.cpp
    UserAgent.cpp
)

qt_add_resources(kompanion_core default_overrides
    PREFIX /kompanion/overrides
    BASE ${PROJECT_SOURCE_DIR}/data/overrides
    FILES
        ${PROJECT_SOURCE_DIR}/data/overrides/generic.css
        ${PROJECT_SOURCE_DIR}/data/overrides/claude.css
        ${PROJECT_SOURCE_DIR}/data/overrides/chatgpt.css
        ${PROJECT_SOURCE_DIR}/data/overrides/copilot.css
        ${PROJECT_SOURCE_DIR}/data/overrides/gemini.css
)

target_include_directories(kompanion_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(kompanion_core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    KF6::ConfigCore
    KF6::I18n
)
