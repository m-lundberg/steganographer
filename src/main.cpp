#include "include/image.hpp"
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
    inputGroup.add_argument("-m", "--message")
        .help("A message string to hide");
    inputGroup.add_argument("-i", "--image")
        .help("Path to an image to hide in the original image");
    hideParser.add_argument("-o", "--output")
        .help("Path to output image, default is '<input>_out.png'");
    hideParser.add_argument("--bpp")
        .help("The number of least significant bits to use in each pixel of the image")
        .scan<'u', size_t>()
        .default_value<size_t>(1);

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
        std::print(std::cerr, "Read image '{}' with dimensions {}x{}x{}\n", path, image.x, image.y, image.channels);

        if (auto msg = hideParser.present("--message")) {
            auto result = hide(image, *msg, hideParser.get<size_t>("--bpp"));
            if (!result) {
                std::print(std::cerr, "Could not hide message: {}\n", result.error());
                return 1;
            }
        }
        else if (auto hidepath = hideParser.present("--image")) {
            const Image hidden(hidepath->c_str());
            std::print(std::cerr, "Read image '{}' with dimensions {}x{}x{}\n", *hidepath, hidden.x, hidden.y, hidden.channels); 

            const auto enc = hidden.encodeString();
            const std::string_view sv(reinterpret_cast<const char*>(enc.data()), enc.size());
            auto result = hide(image, sv, hideParser.get<size_t>("--bpp"));
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
    else if (parser.is_subcommand_used("reveal")) {
        const std::string path = revealParser.get("file");
        const Image image(path.c_str());
        std::print(std::cerr, "Read image '{}' with dimensions {}x{}x{}\n", path, image.x, image.y, image.channels);
        const size_t bpp = revealParser.get<size_t>("--bpp");

        if (revealParser.get("--type") == "string") {
            const size_t length = revealParser.get<size_t>("--length");
            const auto revealed = reveal(image, length, bpp);
            if (!revealed) {
                std::print(std::cerr, "Could not extract string from image: {}\n", revealed.error());
            }
            std::print(std::cerr, "Extracted message: '{}'\n", *revealed);
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
            std::print(std::cerr, "Read image size {}={}x{}x{}\n", imageSize, ints[0], ints[1], ints[2]);

            auto revealedImageData = reveal(image, imageSize, bpp);
            if (!revealedImageData) {
                std::print(std::cerr, "Could not extract image data: {}\n", revealedImageData.error());
                return 1;
            }

            Image revealedImage;
            revealedImage.data = reinterpret_cast<u8*>(revealedImageData->data());
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
    else {
        parser.print_help();
    }

    return 0;
}
