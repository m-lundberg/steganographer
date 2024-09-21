# Interface target collecting third party header-only libs
add_library(thirdparty INTERFACE
    "argparse.hpp"
    "stb_image.h"
    "stb_image_write.h"
)

target_include_directories(thirdparty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
