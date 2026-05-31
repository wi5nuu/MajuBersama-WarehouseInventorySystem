add_library(common_lib
    result.hpp
    logger.hpp
    config.hpp
)

target_include_directories(common_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_lib PUBLIC spdlog::spdlog yaml-cpp fmt::fmt)
