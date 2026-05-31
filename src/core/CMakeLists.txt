add_library(core_lib
    entities/item.hpp
    entities/user.hpp
    entities/warehouse.hpp
    entities/category.hpp
    entities/supplier.hpp
    value_objects/money.hpp
    value_objects/quantity.hpp
    value_objects/address.hpp
    value_objects/email.hpp
    repositories/item_repository.hpp
    repositories/user_repository.hpp
    repositories/warehouse_repository.hpp
    services/auth_service.hpp
    services/inventory_service.hpp
    use_cases/create_item_use_case.hpp
    use_cases/create_item_use_case.cpp
    use_cases/get_item_use_case.hpp
    use_cases/get_item_use_case.cpp
    use_cases/list_items_use_case.hpp
    use_cases/list_items_use_case.cpp
    use_cases/update_item_use_case.hpp
    use_cases/update_item_use_case.cpp
    use_cases/delete_item_use_case.hpp
    use_cases/delete_item_use_case.cpp
    use_cases/authenticate_user_use_case.hpp
    use_cases/authenticate_user_use_case.cpp
)

target_include_directories(core_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_lib PUBLIC common_lib)
