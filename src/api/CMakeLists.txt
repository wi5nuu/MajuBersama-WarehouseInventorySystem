add_library(api_lib
    controllers/item_controller.hpp
    controllers/item_controller.cpp
    controllers/auth_controller.hpp
    controllers/auth_controller.cpp
    middleware/auth_middleware.hpp
    middleware/auth_middleware.cpp
    middleware/logging_middleware.hpp
    middleware/logging_middleware.cpp
    dto/item_dto.hpp
    dto/auth_dto.hpp
    dto/error_response.hpp
    validators/item_validator.hpp
    validators/item_validator.cpp
)

target_include_directories(api_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(api_lib PUBLIC infrastructure_lib core_lib common_lib Drogon::Drogon nlohmann_json::nlohmann_json)
