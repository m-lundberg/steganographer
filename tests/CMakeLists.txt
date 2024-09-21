
add_executable(tests
    "tests.cpp"

    "doctest.h"
)

target_compile_features(tests PUBLIC cxx_std_23)

if(MSVC)
    target_compile_definitions(steganographer PUBLIC UNICODE _UNICODE)
endif()

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
target_link_libraries(tests thirdparty)

install(TARGETS tests)
