// This is the entry point for the gifgen application.
//
// See CMake.txt for details about the build.
// See README.txt for details about the project and how 
// to run this code.

#include <cassert>
#include <iostream>
#include "args.hpp"
#include "image_io.hpp"
#include "image_utils.hpp"
#include "palettize.hpp"

// Get a test file's relative path from its base name
std::string get_image(std::string filename) {
    const std::string test_files_dir = "../test_images/";
    return test_files_dir + filename;
}

std::string file_type_extension(args::input_file_type file_type) {
    if (file_type == args::JPEG) {
        return ".jpeg";
    }
    else {
        assert (file_type == args::PNG);
        return ".png";
    }
}

// Read in a JPEG or PNG image and return its GIL representation.
image::rgb_image_t read_image (const std::string& filename, args::input_file_type file_type) {
    image::rgb_image_t img;
    if (file_type == args::input_file_type::PNG) {
        image::read_png_image(filename, img);
    }
    else {
        assert (file_type == args::input_file_type::JPEG);
        image::read_jpeg_image(filename, img);
    }

    std::cout << "Read in image with dimensions " << img.width() << "x" << img.height() << std::endl;

    return img;
}

// Writes a JPEG or PNG image to the given file.
void write_image(const std::string& filename, 
                 const image::rgb_image_t& img, 
                 args::input_file_type file_type) {
    if (file_type == args::input_file_type::PNG) {
        image::write_png_image(filename, img);
    }
    else {
        assert (file_type == args::input_file_type::JPEG);
        image::write_jpeg_image(filename, img);
    }
}

// Changes each pixel in the image view to the closest
// color in the color table.
template <class ImageView, class ColorTable>
void quantize_image(const ImageView& image_view, 
                    const ColorTable& color_table) {
    auto w = image_view.width();
    auto h = image_view.height();
    
    std::cout << "Quantizing image using color table of " << color_table.size() << " colors" << std::endl;

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            image_view(x, y) = color_table.get_nearest_color(image_view(x, y)).second;
        }
    }
}

int main(int argc, char **argv) {
    auto args = args::parse_arguments(argc, argv);

    if (!args.other_args.empty()) {
        std::cout << "Extra string arguments: ";
        for (const auto& s : args.other_args) {
            std::cout << s << " ";
        }
        std::cout << std::endl;
    }

    // TODO Add logic to check the validity of the given file
    //  - Check that it exists
    //  - Check that it is a jpeg/png image as appropriate

    // Read in the image
    auto img = read_image(args.file_name, args.file_type);
    auto image_view = boost::gil::view(img);

    // Quantize/palettize the image
    auto color_table = palettize::create_color_table(image_view);
    // auto index_list = palettize::palettize_image(image_view, color_table); // TODO

    // As a temporary step, quantize the image in-place and save it to disk.
    quantize_image(image_view, color_table);
    std::string result_file = "quantized_result" + file_type_extension(args.file_type);
    write_image(result_file, img, args.file_type);

    return 0;
}