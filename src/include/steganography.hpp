#ifndef STEGANOGRAPHER_STEGANOGRAPHY_HPP
#define STEGANOGRAPHER_STEGANOGRAPHY_HPP

#include "image.hpp"

#include <expected>
#include <string_view>


std::expected<void, std::string> hide(Image& plainsight, std::string_view message, size_t bpp = 1)
{
    if (bpp > 8) {
        throw std::invalid_argument(std::format("Invalid bpp: {}, must be 0-7", bpp));
    }

    if (message.size() * 8 > plainsight.size() * bpp) {
        return std::unexpected(
            std::format("Could not fit message ({} bytes) in image ({} bytes) using {} LSB",
            message.size(), plainsight.size(), bpp));
    }

    size_t globalBitIndex = 0; // Which bit we are at in the input string

    for (char c : message) { // For each char in input string
        for (size_t bitIndex = 0; bitIndex < 8; ++bitIndex) { // For every bit in current char
            const size_t pixelIndex = globalBitIndex / bpp; // Which pixel this bit should be in
            const size_t bitInPixel = globalBitIndex % bpp; // Which index in the pixel this bit belongs to

            // Extract value of current bit in input string
            const bool bitValue = (c >> bitIndex) & 1;

            // Set the bit at bitInPixel to bitValue in the pixel
            u8& pixel = plainsight.data[pixelIndex];
            pixel = (pixel & ~(1 << bitInPixel)) | (bitValue << bitInPixel);

            globalBitIndex++;
        }
    }

    return {};
}

std::expected<std::string, std::string> reveal(const Image& plainsight, size_t messageLength, size_t bpp = 1)
{
    if (bpp > 8) {
        throw std::invalid_argument(std::format("Invalid bpp: {}, must be 0-7", bpp));
    }

    if (messageLength * 8 > plainsight.size() * bpp) {
        return std::unexpected(
            std::format("Can not extract message of {} bytes from image of {} bytes using {} LSB",
                        messageLength, plainsight.size(), bpp));
    }

    std::string message;
    message.reserve(messageLength);

    size_t globalBitIndex = 0; // Which bit we are at in the input string
    char currentChar = 0;

    for (size_t i = 0; i < messageLength; ++i) {
        for (size_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
            const size_t pixelIndex = globalBitIndex / bpp;
            const size_t bitInPixel = globalBitIndex % bpp;

            // Extract the bit at bitInPixel from the pixel
            const u8 pixel = plainsight.data[pixelIndex];
            const bool bitValue = (pixel >> bitInPixel) & 1;

            // Set the bit at bitIndex in currentChar to bitValue
            currentChar = (currentChar & ~(1 << bitIndex)) | (bitValue << bitIndex);

            globalBitIndex++;
        }

        message += currentChar;
        currentChar = 0;
    }

    return message;
}

#endif // STEGANOGRAPHER_STEGANOGRAPHY_HPP
