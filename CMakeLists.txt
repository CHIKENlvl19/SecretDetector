cmake_minimum_required(VERSION 3.16)
project(SecretDetector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Version
set(SECRET_DETECTOR_VERSION "1.0.0")
add_definitions(-DVERSION="${SECRET_DETECTOR_VERSION}")

# DEPENDENCIES

# nlohmann/json
find_package(nlohmann_json REQUIRED)

# spdlog
find_package(spdlog REQUIRED)

# Threads (для многопоточности)
find_package(Threads REQUIRED)

# Optional: Boost for some utilities
find_package(Boost COMPONENTS system filesystem QUIET)

# INCLUDES

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# SOURCE FILES

set(CORE_SOURCES
    src/core/pattern_matcher.cpp
    src/core/entropy_analyzer.cpp
    src/core/file_scanner.cpp
    src/core/secret_detector.cpp
)

set(UTILS_SOURCES
    src/utils/config_manager.cpp
    src/utils/logger.cpp
    src/utils/file_utils.cpp
    src/utils/export_manager.cpp
)

set(CLI_SOURCES
    src/cli/cli_parser.cpp
    src/main.cpp
)

# CLI VERSION

add_executable(secret_detector ${CORE_SOURCES} ${UTILS_SOURCES} ${CLI_SOURCES})

target_link_libraries(secret_detector
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Threads::Threads
)

if(Boost_FOUND)
    target_link_libraries(secret_detector
        PRIVATE
            Boost::system
            Boost::filesystem
    )
endif()

# GUI VERSION (Qt5)

option(BUILD_GUI "Build GUI version with Qt5" ON)

if(BUILD_GUI)
    find_package(Qt5 COMPONENTS Widgets QUIET)
    
    if(Qt5_FOUND)
        message(STATUS "Qt5 found - building GUI version")
        
        # включить автоматическую обработку Qt
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        
        set(GUI_SOURCES
            src/gui/main_gui.cpp
            src/gui/mainwindow.cpp
            src/gui/mainwindow.h
        )
        
        # GUI executable
        add_executable(secret_detector_gui
            ${CORE_SOURCES}
            ${UTILS_SOURCES}
            ${GUI_SOURCES}
        )
        
        target_link_libraries(secret_detector_gui
            PRIVATE
                Qt5::Widgets
                nlohmann_json::nlohmann_json
                spdlog::spdlog
                Threads::Threads
        )
        
        if(Boost_FOUND)
            target_link_libraries(secret_detector_gui
                PRIVATE
                    Boost::system
                    Boost::filesystem
            )
        endif()
        
        target_include_directories(secret_detector_gui
            PRIVATE
                ${CMAKE_SOURCE_DIR}/src
        )
        
        # установка GUI версии
        install(TARGETS secret_detector_gui DESTINATION bin)
        
    else()
        message(WARNING "Qt5 not found - GUI version will not be built. Install Qt5: sudo pacman -S qt5-base")
    endif()
else()
    message(STATUS "GUI version disabled (BUILD_GUI=OFF)")
endif()

# INSTALLATION

install(TARGETS secret_detector
        RUNTIME DESTINATION bin)

install(FILES config/patterns.json
        DESTINATION etc/secret_detector)

install(FILES README.md LICENSE
        DESTINATION share/doc/secret_detector)

# TESTING

# enable_testing()
# add_subdirectory(tests)

# SUMMARY

message(STATUS "")
message(STATUS "==========")
message(STATUS "Secret Detector Configuration Summary")
message(STATUS "==========")
message(STATUS "Version:          ${SECRET_DETECTOR_VERSION}")
message(STATUS "C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  CLI version:    YES")
message(STATUS "  GUI version:    ${Qt5_FOUND}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  nlohmann_json:  ${nlohmann_json_FOUND}")
message(STATUS "  spdlog:         ${spdlog_FOUND}")
message(STATUS "  Qt5:            ${Qt5_FOUND}")
message(STATUS "  Boost:          ${Boost_FOUND}")
message(STATUS "  Threads:        ${Threads_FOUND}")
message(STATUS "==========")
message(STATUS "")
