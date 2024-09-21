#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <image.hpp>


// For printing in tests
std::ostream& operator<<(std::ostream& os, const Image& img)
{
    std::string data;
    std::copy(img.data, img.data + img.size(), std::back_inserter(data));
    std::print(os, "Image[x={}, y={}, channels={}, data={}]", img.x, img.y, img.channels, data);
    return os;
}


TEST_CASE("Image")
{
    SUBCASE("Encode and decode")
    {
        Image img;
        img.x = 123;
        img.y = 456;
        img.channels = 1;
        img.data = new u8[123 * 456];
        std::fill(img.data, img.data + img.size(), 123);

        auto encoded = img.encodeString();
        auto decoded = Image::decodeString(encoded);
        CHECK(decoded.has_value());
        CHECK(decoded.value() == img);
    }
}
