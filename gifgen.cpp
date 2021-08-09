// This is the entry point for the gifgen application, and
// mostly serves to connect the component parts together. 
//
// See CMake.txt for details about the build.
// See README.txt for details about the project and how 
// to run this code.

#include <cassert>
#include <iostream>
#include <fstream>
#include "args.hpp"
#include "image_io.hpp"
#include "image_utils.hpp"
#include "gif_builder.hpp"

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

    return img;
}

int main(int argc, char **argv) {
    auto args = args::parse_arguments(argc, argv);
    assert (!args.input_files.empty());

    // Read in the first image to get the dimensions. TODO replace this with proper validation as described below.
    auto first_img = read_image(args.input_files.at(0), args.file_type);
    auto width = first_img.width();
    auto height = first_img.height();

    // Create the GIF data stream
    std::ofstream output_file(args.output_file_name, std::ios::out | std::ios::binary);
    gif::gif_builder gif_stream(output_file, width, height, args.delay);

    // Add each frame to the GIF
    for (const auto& filename : args.input_files) {
        std::cout << "Adding frame '" << filename << "' to " << args.output_file_name << std::endl;

        auto img = read_image(filename, args.file_type);
        auto image_view = boost::gil::view(img);
        gif_stream.add_frame(image_view);
    }
    
    gif_stream.complete_stream();
    
    // TODO Add logic to check the validity of the given files
    //    Check that the input files exist, are of the appropriate type (jpeg/png),
    //    and have the same dimensions

    return 0;
}