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
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Widgets Xml)
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
            qml/screens/MainScreen.qml
            qml/screens/SettingsScreen.qml
            qml/screens/ActivityScreen.qml
            qml/screens/UninstallScreen.qml
            qml/components/ModePill.qml
            qml/components/FriendRow.qml
            qml/components/NeonBtn.qml
        RESOURCES
            resources/icons/tray.svg
    )

    target_include_directories(nyx PRIVATE src third_party)

    target_link_libraries(nyx PRIVATE
        Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Network Qt6::Widgets Qt6::Xml
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

        # windeployqt: bundle Qt DLLs + QML plugins next to nyx.exe so the
        # output dir is self-contained. Run after every link.
        find_program(WINDEPLOYQT windeployqt HINTS ${Qt6_DIR}/../../../bin REQUIRED)
        add_custom_command(TARGET nyx POST_BUILD
            COMMAND "${WINDEPLOYQT}"
                    --qmldir "${CMAKE_SOURCE_DIR}/qml"
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    --release
                    "$<TARGET_FILE:nyx>"
            COMMENT "windeployqt: bundling Qt runtime")

        # Bundle OpenSSL DLLs alongside if found. Qt's openssl backend dlopens
        # libssl-3-x64.dll / libcrypto-3-x64.dll at runtime.
        if(OPENSSL_FOUND)
            get_filename_component(OPENSSL_DIR "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
            foreach(dll libssl-3-x64.dll libcrypto-3-x64.dll)
                if(EXISTS "${OPENSSL_DIR}/bin/${dll}")
                    add_custom_command(TARGET nyx POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                "${OPENSSL_DIR}/bin/${dll}"
                                "$<TARGET_FILE_DIR:nyx>"
                        COMMENT "copy ${dll}")
                endif()
            endforeach()
        endif()
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

    if(NYX_BUILD_APP)
        add_executable(nyx_xmpp_smoke
            tests/xmpp_smoke.cpp
            src/core/xmpp.cpp
        )
        target_include_directories(nyx_xmpp_smoke PRIVATE src)
        target_link_libraries(nyx_xmpp_smoke PRIVATE Qt6::Core Qt6::Xml)
        enable_testing()
        add_test(NAME xmpp_smoke COMMAND nyx_xmpp_smoke)
    endif()

    enable_testing()
    add_test(NAME cert_smoke COMMAND nyx_cert_smoke)
endif()
