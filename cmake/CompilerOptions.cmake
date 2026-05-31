if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
    add_compile_options(-Werror=return-type -Werror=uninitialized)
    add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-DNDEBUG -flto)
    add_link_options(-flto)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
