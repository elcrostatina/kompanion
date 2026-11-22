# SPDX-FileCopyrightText: 2026 Kompanion contributors
# SPDX-License-Identifier: GPL-3.0-or-later

function(kompanion_add_test name)
    qt_add_executable(${name} ${name}.cpp ${ARGN})
    target_link_libraries(${name} PRIVATE kompanion_core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

kompanion_add_test(tst_openaiclient)
kompanion_add_test(tst_chatstore)
kompanion_add_test(tst_providerregistry)
kompanion_add_test(tst_appsettings)
kompanion_add_test(tst_webhelpers)
kompanion_add_test(tst_overridescripts)
target_link_libraries(tst_overridescripts PRIVATE Qt6::Qml)

kompanion_add_test(tst_chatcontroller
    ${PROJECT_SOURCE_DIR}/src/app/ChatController.cpp
    ${PROJECT_SOURCE_DIR}/src/app/ChatListModel.cpp
    ${PROJECT_SOURCE_DIR}/src/app/MessageModel.cpp
)
target_link_libraries(tst_chatcontroller PRIVATE Qt6::Qml)
target_include_directories(tst_chatcontroller PRIVATE ${PROJECT_SOURCE_DIR}/src/app)
