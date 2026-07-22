cmake_minimum_required(VERSION 3.16)

project(MinecraftLauncher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Core)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    core/versionmanager.cpp
    core/versionmanager.h
    core/gamedownloader.cpp
    core/gamedownloader.h
    core/gamelauncher.cpp
    core/gamelauncher.h
    core/configmanager.cpp
    core/configmanager.h
    core/skinmanager.cpp
    core/skinmanager.h
    core/resourcemanager.cpp
    core/resourcemanager.h
    ui/settingstab.cpp
    ui/settingstab.h
    ui/settingstab.ui
    ui/skintab.cpp
    ui/skintab.h
    ui/skintab.ui
    ui/resourcetab.cpp
    ui/resourcetab.h
    ui/resourcetab.ui
    ui/addaccountdialog.cpp
    ui/addaccountdialog.h
    ui/addaccountdialog.ui
    resources.qrc
)

add_executable(MinecraftLauncher
    ${PROJECT_SOURCES}
)

target_link_libraries(MinecraftLauncher PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
)

set_target_properties(MinecraftLauncher PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)