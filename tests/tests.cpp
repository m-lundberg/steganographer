#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <compression.hpp>
#include <image.hpp>
#include <int_types.hpp>


// For printing in tests
std::ostream& operator<<(std::ostream& os, const Image& img)
{
    std::string data;
    std::copy(img.data, img.data + img.size(), std::back_inserter(data));
    std::print(os, "Image[x={}, y={}, channels={}, data={}]", img.x, img.y, img.channels, data);
    return os;
}


TEST_CASE("Image encode and decode")
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

TEST_CASE("RLE compression")
{
    CHECK(rle::extract(rle::compress("")) == "");
    CHECK(rle::extract(rle::compress("a")) == "a");
    CHECK(rle::extract(rle::compress("abc")) == "abc");
    CHECK(rle::extract(rle::compress("aaabbbccc")) == "aaabbbccc");
    CHECK(rle::extract(rle::compress("aaaaaaaaaabbbbbbbbbbcccccccccc")) == "aaaaaaaaaabbbbbbbbbbcccccccccc");
    CHECK(rle::extract(rle::compress("ccccc")) == "ccccc");
    CHECK(rle::extract(rle::compress("1234")) == "1234");

    CHECK(rle::extract<u8>(rle::compress<u8>("aaaabbbbbccccc")) == "aaaabbbbbccccc");
    CHECK(rle::extract<u16>(rle::compress<u16>("aaaabbbbbccccc")) == "aaaabbbbbccccc");
    CHECK(rle::extract<u32>(rle::compress<u32>("aaaabbbbbccccc")) == "aaaabbbbbccccc");
}
