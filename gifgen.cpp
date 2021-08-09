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

    std::cout << "Read in image with dimensions " << img.width() << "x" << img.height() << std::endl;

    return img;
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

    // TODO Add logic to check the validity of the given files
    //    Check that the input files exist, are of the appropriate type (jpeg/png),
    //    and have the same dimensions

    // Read in the image
    auto img = read_image(args.file_name, args.file_type);
    auto image_view = boost::gil::view(img);

    // Open the output file and write the single-image data
    std::ofstream output_file(args.output_file_name, std::ios::out | std::ios::binary);

    gif::gif_builder gif_stream(output_file, image_view.width(), image_view.height());
    gif_stream.add_frame(image_view);
    gif_stream.complete_stream();

    return 0;
}