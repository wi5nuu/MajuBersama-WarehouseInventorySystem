cmake_minimum_required(VERSION 3.27)
project(inventory_management_system VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(libpqxx REQUIRED)
find_package(Drogon REQUIRED)
find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(nlohmann_json REQUIRED)

# jwt-cpp is header-only; locate it via find_path or fetch
if(NOT TARGET jwt-cpp::jwt-cpp)
    find_path(JWT_CPP_INCLUDE_DIR jwt-cpp/jwt.h
        HINTS /usr/include /usr/local/include /opt/homebrew/include
    )
    if(JWT_CPP_INCLUDE_DIR)
        add_library(jwt-cpp::jwt-cpp INTERFACE IMPORTED)
        target_include_directories(jwt-cpp::jwt-cpp INTERFACE "${JWT_CPP_INCLUDE_DIR}")
        target_link_libraries(jwt-cpp::jwt-cpp INTERFACE OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json)
    else()
        message(WARNING "jwt-cpp not found, downloading...")
        include(FetchContent)
        FetchContent_Declare(jwt-cpp
            GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
            GIT_TAG v0.7.0
        )
        FetchContent_MakeAvailable(jwt-cpp)
    endif()
endif()

add_subdirectory(src/core)
add_subdirectory(src/infrastructure)
add_subdirectory(src/api)
add_subdirectory(src/common)
add_subdirectory(tests)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_lib
        infrastructure_lib
        api_lib
        common_lib
        fmt::fmt
        spdlog::spdlog
        libpqxx::libpqxx
        Drogon::Drogon
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        jwt-cpp::jwt-cpp
)
