// This is the entry point for the gifgen application, and
// mostly serves to connect the component parts together. 
//
// See CMake.txt for details about the build.
// See README.txt for details about the project and how 
// to run this code.

#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "args.hpp"
#include "image_io.hpp"
#include "image_utils.hpp"
#include "gif_builder.hpp"

struct image_dims {
    std::size_t width;
    std::size_t height;
};

// Returns true iff:
//  1. All filenames in the list correspond to files of the given type, and
//  2. All files are of the same dimensions, and
//  3. The specific encoding can be read using a 24-bit
//     color space (8 bits per channel)
//
// On success, the dims parameter is updated to hold the dimensions of the
// input frames.
//
// Error information will be printed before returning false.
bool check_images_are_compatible(const std::vector<std::string>& filenames, 
                                 image::file_type type, 
                                 image_dims& dims) {
    assert (!filenames.empty());
    
    dims.width = 0;
    dims.height = 0;

    try {
        const std::string& first_file_name = filenames.front();
        image::rgb_image_t img;
        for (const auto& filename : filenames) {
            if (!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename)) {
                std::cout << "Error: Unable to find input file " << filename << std::endl;
                return false;
            }
            else if (!image::is_file_type(filename, type)) {
                std::cout << "Error: Input file " << filename << " does not match specified input file type." << std::endl;
                return false;
            }
            else {
                // Unfortunately, to verify that the image can be represented using 
                // 8-bit color channels, we need to read in entire images here. This
                // causes duplication with the main processing loop, but this can't be 
                // avoided without deferring failure on later frames until after we"ve
                // encoded their predecessors.
                image::read_image(filename, img, type);
                auto img_view = boost::gil::view(img);

                assert (img_view.width() > 0);
                assert (img_view.height() > 0);
                auto img_width = static_cast<std::size_t>(img_view.width());
                auto img_height = static_cast<std::size_t>(img_view.height());

                if (dims.width == 0 && dims.height == 0) {
                    // First image, record its dimensions.
                    dims.width = img_width;
                    dims.height = img_height;
                }
                else if (dims.width != img_width || 
                         dims.height != img_height) {
                    std::cout << "Error: frames " << first_file_name << " and " << filename << " have differing dimensions." << std::endl;
                    return false;
                }
            }
        }
    }
    catch(std::exception& e) {
        // There may be many possible file handling issues (permissions, symlinks,
        // etc., so we return false if anything goes wrong and leave it to the user
        // to diagnose these sort of issues.
        std::cout << "Error: failed to read input files. Operation failed with message: " 
                  <<  e.what() 
                  << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
    auto args = args::parse_arguments(argc, argv);

    // In the interest of detecting user errors as quickly as possible, 
    // we check that we are able to read in each input frame and that 
    // they all match in size before doing any real processing. 
    // We also determine the dimensions at the same time. 
    assert (!args.input_files.empty());
    image_dims dims {0, 0};
    if (!check_images_are_compatible(args.input_files, args.file_type, dims)) {
        return 1;
    }

    // Create the GIF data stream
    std::ofstream output_file(args.output_file_name, std::ios::out | std::ios::binary);
    gif::gif_builder gif_stream(output_file, dims.width, dims.height, args.delay);

    // Add each frame to the GIF
    image::rgb_image_t img;
    for (const auto& filename : args.input_files) {
        std::cout << "Adding frame '" << filename << "' to " << args.output_file_name << std::endl;
        
        image::read_image(filename, img, args.file_type);
        auto image_view = boost::gil::view(img);
        gif_stream.add_frame(image_view);

        std::cout << std::endl;
    }
    
    gif_stream.complete_stream();
    
    std::cout << "GIF file " << args.output_file_name 
              << " created with " << args.input_files.size() << " frame(s)" 
              << std::endl;

    return 0;
}