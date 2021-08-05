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

// Changes each pixel in the image view to the quantized value.
void recreate_quantized_image(const image::rgb_image_view_t& image_view, 
                              const palettize::color_table& palette,
                              std::vector<uint8_t> indices) {    
    std::cout << "Re-creating quantized image from indices into palette of " << palette.size() << " colors" << std::endl;

    auto w = image_view.width();
    auto h = image_view.height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int i = w * y + x; // Index into index list
            auto table_index = indices.at(i); // Index into palette
            image_view(x, y) = palette.at(table_index);
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
    auto index_list = palettize::palettize_image(image_view, color_table);

    // As a temporary step, re-create the quantized image from the index 
    // list and the color table as a sanity check.
    recreate_quantized_image(image_view, color_table, index_list);
    std::string result_file = "quantized_result" + file_type_extension(args.file_type);
    write_image(result_file, img, args.file_type);

    return 0;
}