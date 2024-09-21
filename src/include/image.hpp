#ifndef STEGANOGRAPHER_IMAGE_HPP
#define STEGANOGRAPHER_IMAGE_HPP

#include <algorithm>
#include <cstdint>
#include <expected>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include <stb_image_write.h>


using u8 = uint8_t;
using i32 = int32_t;


struct Image {
    Image() = default;
    Image(const char* filename) : ownsData(true) {
        data = stbi_load(filename, &x, &y, &channels, 0);
    }
    ~Image() {
        if (ownsData && data) {
            stbi_image_free(data);
        }
        data = nullptr;
    }

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& rhs) noexcept
        : x(rhs.x), y(rhs.y), channels(rhs.channels), data(rhs.data) {
        rhs.data = nullptr;
    }
    Image& operator=(Image&& rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        if (ownsData && data) {
            stbi_image_free(data);
        }
        x = rhs.x;
        y = rhs.y;
        channels = rhs.channels;
        data = rhs.data;
        rhs.data = nullptr;
        return *this;
    }

    size_t size() const { return x * y * channels; }

    bool operator==(const Image& rhs) const {
        return x == rhs.x && y == rhs.y && channels == rhs.channels && std::equal(data, data + size(), rhs.data);
    }

    // Save the image to file, guessing the file type by the filename. Supported formats: png, bmp, jpg.
    std::expected<void, std::string> save(const char* filename) {
        std::string_view path(filename);
        if (path.ends_with(".png")) {
            if (stbi_write_png(filename, x, y, channels, data, 0) == 0) {
                return std::unexpected(std::format("Could not save png to path {}", path));
            }
            return {};
        }
        else if (path.ends_with(".bmp")) {
            if (stbi_write_bmp(filename, x, y, channels, data) == 0) {
                return std::unexpected(std::format("Could not save bmp to path {}", path));
            }
            return {};
        }
        else if (path.ends_with(".jpg")) {
            if (stbi_write_jpg(filename, x, y, channels, data, 100) == 0) {
                return std::unexpected(std::format("Could not save jpg to path {}", path));
            }
            return {};
        }
        return std::unexpected(std::format("Could not guess image type from extension for {}", path));
    }

    // Encode the image into a string representation
    std::string encodeString() const {
        std::string result(12, 0);

        // Store image size as ints in first few bytes
        {
            i32* ints = reinterpret_cast<i32*>(result.data());
            ints[0] = x;
            ints[1] = y;
            ints[2] = channels;
        }

        for (int i = 0; i < size(); ++i) {
            result.push_back(data[i]);
        }
        return result;
    }

    // Decode a string representation created by encodeString() into a new Image
    static std::expected<Image, std::string> decodeString(std::string_view str) {
        if (str.size() < 12) {
            // Need at least 3 i32s for image size
            return std::unexpected("Not enough data in string for image size");
        }

        Image result;
        {
            const i32* ints = reinterpret_cast<const i32*>(str.data());
            result.x = ints[0];
            result.y = ints[1];
            result.channels = ints[2];
        }

        if (str.size() - 12 < result.size()) {
            return std::unexpected("Not enough data in string to decode image");
        }

        result.data = new u8[result.size()];
        for (size_t i = 0; i < result.size(); ++i) {
            result.data[i] = str[i + 12];
        }

        return result;
    }

    int x = 0;
    int y = 0;
    int channels = 0;
    uint8_t* data = nullptr;

  private:
    // If the memory pointed to by data is owned by this class or not
    bool ownsData = false;
};

#endif // STEGANOGRAPHER_IMAGE_HPP
