cmake_minimum_required(VERSION 3.15)

project(
    steganographer
    VERSION 1.0.0
    LANGUAGES CXX
)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc")
endif(MSVC)


add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(tests)
