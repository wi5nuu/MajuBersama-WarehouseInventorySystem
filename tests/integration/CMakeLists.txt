add_executable(test_db_connection test_db_connection.cpp)
target_link_libraries(test_db_connection PRIVATE core_lib common_lib infrastructure_lib GTest::GTest GTest::Main pqxx::pqxx)
target_include_directories(test_db_connection PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test_db_connection COMMAND test_db_connection)

# Add more integration tests here
