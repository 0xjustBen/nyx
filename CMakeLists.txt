cmake_minimum_required(VERSION 3.21)
project(nyx VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(NYX_BUILD_APP "Build Qt GUI app" ON)
option(NYX_BUILD_TESTS "Build smoke tests" OFF)

if(NOT WIN32)
    find_package(OpenSSL REQUIRED)
endif()

if(NYX_BUILD_APP)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Widgets)
    qt_standard_project_setup(REQUIRES 6.5)
    qt_policy(SET QTP0004 NEW)
endif()

set(NYX_CORE_SOURCES
    src/core/proxy.cpp
    src/core/xmpp.cpp
    src/core/cert.cpp
    src/core/riot_paths.cpp
    src/core/config.cpp
    src/core/config_proxy.cpp
    src/core/launcher.cpp
)

set(NYX_PLATFORM_SOURCES
    $<$<PLATFORM_ID:Darwin>:src/platform/trust_mac.mm>
    $<$<PLATFORM_ID:Windows>:src/platform/trust_win.cpp>
    $<$<PLATFORM_ID:Linux>:src/platform/trust_linux.cpp>
)

set(NYX_UI_SOURCES
    src/ui/app_controller.cpp
    src/ui/presence_model.cpp
    src/ui/roster_model.cpp
)

if(NYX_BUILD_APP)
    qt_add_executable(nyx
        src/main.cpp
        ${NYX_CORE_SOURCES}
        ${NYX_PLATFORM_SOURCES}
        ${NYX_UI_SOURCES}
    )

    set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

    qt_add_qml_module(nyx
        URI Nyx
        VERSION 1.0
        QML_FILES
            qml/Main.qml
            qml/Theme.qml
            qml/screens/StatusScreen.qml
            qml/screens/SettingsScreen.qml
            qml/screens/RosterScreen.qml
            qml/components/ModePill.qml
            qml/components/FriendRow.qml
            qml/components/TitleBar.qml
        RESOURCES
            resources/icons/tray.svg
    )

    target_include_directories(nyx PRIVATE src third_party)

    target_link_libraries(nyx PRIVATE
        Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Network Qt6::Widgets
    )
    if(NOT WIN32)
        target_link_libraries(nyx PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()

    if(APPLE)
        target_link_libraries(nyx PRIVATE "-framework Security" "-framework CoreFoundation")
        set_target_properties(nyx PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER io.nyx.app
            MACOSX_BUNDLE_BUNDLE_NAME "Nyx"
        )
    elseif(WIN32)
        target_link_libraries(nyx PRIVATE crypt32 ws2_32)
        set_target_properties(nyx PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()

    include(GNUInstallDirs)
    install(TARGETS nyx
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(NYX_BUILD_TESTS AND NOT WIN32)
    add_executable(nyx_cert_smoke
        tests/cert_smoke.cpp
        src/core/cert.cpp
        src/core/riot_paths.cpp
    )
    target_include_directories(nyx_cert_smoke PRIVATE src)
    target_link_libraries(nyx_cert_smoke PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    enable_testing()
    add_test(NAME cert_smoke COMMAND nyx_cert_smoke)
endif()
