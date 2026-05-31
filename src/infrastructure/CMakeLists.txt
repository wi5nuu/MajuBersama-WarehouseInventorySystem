add_library(infrastructure_lib
    database/connection_pool.hpp
    database/connection_pool.cpp
    database/migration.hpp
    database/migration.cpp
    repositories/postgres_item_repository.hpp
    repositories/postgres_item_repository.cpp
    repositories/postgres_user_repository.hpp
    repositories/postgres_user_repository.cpp
    cache/cache_adapter.hpp
    cache/cache_adapter.cpp
    messaging/message_broker.hpp
    messaging/message_broker.cpp
)

target_include_directories(infrastructure_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(infrastructure_lib PUBLIC core_lib common_lib pqxx::pqxx OpenSSL::SSL OpenSSL::Crypto jwt-cpp::jwt-cpp)
