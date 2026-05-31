set(UNIT_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_money.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_quantity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_email.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_address.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_uuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_item_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_user_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_create_item_use_case.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_get_item_use_case.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_list_items_use_case.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_update_item_use_case.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_delete_item_use_case.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_authenticate_user_use_case.cpp
)

foreach(src ${UNIT_TEST_SOURCES})
    get_filename_component(test_name ${src} NAME_WE)
    add_executable(${test_name} ${src})
    target_link_libraries(${test_name}
        PRIVATE
            core_lib
            common_lib
            GTest::GTest
            GTest::Main
    )
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 30)
endforeach()
