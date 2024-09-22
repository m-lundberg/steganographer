#ifndef STEGANOGRAPHER_COMPRESSION_HPP
#define STEGANOGRAPHER_COMPRESSION_HPP

#include "int_types.hpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>


namespace rle {

// Create an RLE compressed string corresponding to the input
template<typename CountT = u16>
requires(std::is_integral_v<CountT>)
std::string compress(std::string_view data)
{
    std::string result;

    // Helper to store count and then the character in the result string
    const auto storeChar = [&result](char c, CountT count) {
        result.resize(result.size() + sizeof(CountT));                                         // Add bytes to store a number (of type CountT)
        CountT* u = reinterpret_cast<CountT*>(result.data() + result.size() - sizeof(CountT)); // One past the last char
        *u = count;
        result += c;
    };

    char current = 0;
    CountT count = 0;

    for (char c : data) {
        if (c != current && count > 0) {
            storeChar(current, count);
            count = 0;
        }

        current = c;
        assert(count < std::numeric_limits<CountT>::max());
        count++;
    }

    if (count > 0) {
        storeChar(current, count);
    }

    return result;
}

// Extract an RLE compressed string into the original uncompressed string
template<typename CountT = u16>
requires(std::is_integral_v<CountT>)
std::string extract(std::string_view data)
{
    std::string result;

    for (size_t i = 0; i < data.size(); ++i) {
        const CountT* count = reinterpret_cast<const CountT*>(data.data() + i);
        i += sizeof(CountT); // Skip past the count
        char c = data[i];
        for (CountT j = 0; j < *count; ++j) {
            result.push_back(c);
        }
    }

    return result;
}

}

#endif // STEGANOGRAPHER_COMPRESSION_HPP
