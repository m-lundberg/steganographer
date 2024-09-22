#include "include/compression.hpp"
#include "include/image.hpp"
#include "include/int_types.hpp"
#include "include/steganography.hpp"

#include <argparse.hpp>

#include <format>
#include <iostream>


int main(int argc, char* argv[])
{
    argparse::ArgumentParser parser("steganographer", "1.0.0");

    argparse::ArgumentParser hideParser("hide");
    parser.add_subparser(hideParser);
    hideParser.add_description("Hide something in an image");
    hideParser.add_argument("file")
        .help("Path to image to hide data in")
        .required();
    auto& inputGroup = hideParser.add_mutually_exclusive_group(true);
    inputGroup.add_argument("-s", "--string")
        .help("A message string to hide");
    inputGroup.add_argument("-i", "--image")
        .help("Path to an image to hide in the original image");
    hideParser.add_argument("-o", "--output")
        .help("Path to output image, default is '<input>_out.png'");
    hideParser.add_argument("--bpp")
        .help("The number of least significant bits to use in each pixel of the image")
        .scan<'u', size_t>()
        .default_value<size_t>(1);
    hideParser.add_argument("--rle")
        .help("Apply run length encoding using the specified number of bytes to store the count "
              "of each character to input before storing it")
        .scan<'u', u32>()
        .choices(1u, 2u, 4u, 8u);

    argparse::ArgumentParser revealParser("reveal");
    parser.add_subparser(revealParser);
    revealParser.add_description("Extract a hidden message from an image");
    revealParser.add_argument("file")
        .help("Path to an image to extract data from")
        .required();
    revealParser.add_argument("-t", "--type")
        .help("If the extracted data should be printed directly (string) or saved as an image")
        .default_value(std::string("string"))
        .choices("string", "image");
    revealParser.add_argument("-l", "--length")
        .help("The number of bytes to extract from the image (not needed with --type image)")
        .scan<'u', size_t>()
        .default_value<size_t>(0);
    revealParser.add_argument("-o", "--output")
        .help("Path to output image, default is '<input>_out.png'");
    revealParser.add_argument("--bpp")
        .help("The number of least significant bits to use in each pixel of the image")
        .scan<'u', size_t>()
        .default_value<size_t>(1);
    revealParser.add_argument("--rle")
        .help("Extract run length encoded data the specified number of bytes to store the count of each character")
        .scan<'u', u32>()
        .choices(1u, 2u, 4u, 8u);

    try {
        parser.parse_args(argc, argv);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n' << std::endl;
        parser.print_help();
        return 1;
    }

    if (parser.is_subcommand_used("hide")) {
        const std::string path = hideParser.get("file");
        Image image(path.c_str());
        std::print(std::cerr, "Read image '{}' with dimensions {}x{}x{}={}\n",
                   path, image.x, image.y, image.channels, image.x * image.y * image.channels);

        if (auto msg = hideParser.present("--string")) {
            std::string message = *msg;
            std::print(std::cerr, "Message size: {}\n", message.size());

            if (auto rleBytes = hideParser.present<u32>("--rle")) {
                if (*rleBytes == 1) {
                    message = rle::compress<u8>(message);
                }
                else if (*rleBytes == 2) {
                    message = rle::compress<u16>(message);
                }
                else if (*rleBytes == 4) {
                    message = rle::compress<u32>(message);
                }
                else if (*rleBytes == 8) {
                    message = rle::compress<u64>(message);
                }
                std::print(std::cerr, "Size after RLE compression: {}\n", message.size());
            }

            auto result = hide(image, message, hideParser.get<size_t>("--bpp"));
            if (!result) {
                std::print(std::cerr, "Could not hide string: {}\n", result.error());
                return 1;
            }
        }
        else if (auto hidepath = hideParser.present("--image")) {
            const Image hidden(hidepath->c_str());
            std::print(std::cerr, "Read image '{}' with dimensions {}x{}x{}={}\n",
                       *hidepath, hidden.x, hidden.y, hidden.channels, hidden.x * hidden.y * hidden.channels);

            std::string message = hidden.encodeString();

            if (auto rleBytes = hideParser.present<u32>("--rle")) {
                if (*rleBytes == 1) {
                    message = rle::compress<u8>(message);
                }
                else if (*rleBytes == 2) {
                    message = rle::compress<u16>(message);
                }
                else if (*rleBytes == 4) {
                    message = rle::compress<u32>(message);
                }
                else if (*rleBytes == 8) {
                    message = rle::compress<u64>(message);
                }
                std::print(std::cerr, "Size after RLE compression: {}\n", message.size());
            }

            auto result = hide(image, message, hideParser.get<size_t>("--bpp"));
            if (!result) {
                std::print(std::cerr, "Could not hide image: {}\n", result.error());
                return 1;
            }
        }

        std::string outpath = hideParser.present("--output")
                                  ? *hideParser.present("--output")
                                  : path.substr(0, path.find_last_of('.')) + "_out.png";
        image.save(outpath.c_str());
        std::print(std::cerr, "Saved modified image to {}\n", outpath);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else if (parser.is_subcommand_used("reveal")) {
        const std::string path = revealParser.get("file");
        const Image image(path.c_str());
        std::print(std::cerr, "Read image '{}' with dimensions {}x{}x{}={}\n",
                   path, image.x, image.y, image.channels, image.x * image.y * image.channels);
        const size_t bpp = revealParser.get<size_t>("--bpp");

        if (revealParser.get("--type") == "string") {
            const size_t length = revealParser.get<size_t>("--length");
            const auto revealed = reveal(image, length, bpp);
            if (!revealed) {
                std::print(std::cerr, "Could not extract string from image: {}\n", revealed.error());
            }
            std::string message = *revealed;
            std::print(std::cerr, "Extracted message size: {}\n", message.size());

            if (auto rleBytes = revealParser.present<u32>("--rle")) {
                if (*rleBytes == 1) {
                    message = rle::extract<u8>(message);
                }
                else if (*rleBytes == 2) {
                    message = rle::extract<u16>(message);
                }
                else if (*rleBytes == 4) {
                    message = rle::extract<u32>(message);
                }
                else if (*rleBytes == 8) {
                    message = rle::extract<u64>(message);
                }
                std::print(std::cerr, "Size after RLE extraction: {}\n", message.size());
            }

            std::print(std::cerr, "Extracted message: '{}'\n", message);
        }
        else if (revealParser.get("--type") == "image") {
            const size_t bytesForImageSize = 12 * bpp; // 3 i32s = 12 bytes
            const auto imageSizeInfo = reveal(image, bytesForImageSize, bpp);
            if (!imageSizeInfo) {
                std::print(std::cerr, "Could not extract image size: {}\n", imageSizeInfo.error());
                return 1;
            }

            const i32* ints = reinterpret_cast<const i32*>(imageSizeInfo->data());
            const size_t imageSize = ints[0] * ints[1] * ints[2];
            std::print(std::cerr, "Read image size {}x{}x{}={}\n", ints[0], ints[1], ints[2], imageSize);

            auto revealedImageData = reveal(image, imageSize, bpp);
            if (!revealedImageData) {
                std::print(std::cerr, "Could not extract image data: {}\n", revealedImageData.error());
                return 1;
            }
            std::string message = *revealedImageData;

            if (auto rleBytes = revealParser.present<u32>("--rle")) {
                if (*rleBytes == 1) {
                    message = rle::extract<u8>(message);
                }
                else if (*rleBytes == 2) {
                    message = rle::extract<u16>(message);
                }
                else if (*rleBytes == 4) {
                    message = rle::extract<u32>(message);
                }
                else if (*rleBytes == 8) {
                    message = rle::extract<u64>(message);
                }
                std::print(std::cerr, "Size after RLE extraction: {}\n", message.size());
            }

            Image revealedImage;
            revealedImage.data = reinterpret_cast<u8*>(message.data());
            revealedImage.x = ints[0];
            revealedImage.y = ints[1];
            revealedImage.channels = ints[2];

            std::string outpath = revealParser.present("--output")
                                      ? *revealParser.present("--output")
                                      : path.substr(0, path.find_last_of('.')) + "_out.png";
            revealedImage.save(outpath.c_str());
            std::print(std::cerr, "Saved modified image to {}\n", outpath);
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else {
        parser.print_help();
    }

    return 0;
}
