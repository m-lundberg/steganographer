
add_executable(steganographer
    "main.cpp"

    "include/compression.hpp"
    "include/image.hpp"
    "include/int_types.hpp"
    "include/steganography.hpp"
)

target_compile_features(steganographer PUBLIC cxx_std_23)

if(MSVC)
    target_compile_definitions(steganographer PUBLIC UNICODE _UNICODE)
endif()

target_link_libraries(steganographer thirdparty)

install(TARGETS steganographer)
