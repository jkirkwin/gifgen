#include <cassert>
#include <getopt.h>
#include <iostream>
#include "image_io.hpp"

template <class ImageView>
void edit_image(ImageView& view) {
    auto w = view.width();
    auto h = view.height();
    
    // Black out the top-left eighth of the image
    for (int x = 0; x < w/4; ++x) {
        for (int y = 0; y < h/4; ++y) {
            view(x, y) = {0, 0, 0};
        }
    }

    // Red line through the middle, horizontally
    int vertical_midpoint = h/2;
    for (int x = 0; x < w; ++x) {
        view(x, vertical_midpoint - 1) = {0xFF, 0, 0};
        view(x, vertical_midpoint) = {0xFF, 0, 0};
        view(x, vertical_midpoint + 1) = {0xFF, 0, 0};
    }

    // Green line in the middle half of the image, vertically
    int horizontal_midpoint = w/2;
    int vertical_start = vertical_midpoint/2;
    int vertical_end = vertical_start*3;
    for (int y = vertical_start; y < vertical_end; ++y) {
        view(horizontal_midpoint, y) = {0, 0xFF, 0};
    }

    // White out the bottom-right eighth of the image
    for (int x = 3*w/4; x < w; ++x) {
        for (int y = 3*h/4; y < h; ++y) {
            view(x, y) = {0xff, 0xff, 0xff};
        }
    }
}

// Get a test file's relative path from its base name
std::string get_image(std::string filename) {
    const std::string test_files_dir = "../test_images/";
    return test_files_dir + filename;
}

// Draw some lines in the image and write it back to disk
void edit_and_save_png() {
    // Image object used to house the frames being read
    boost::gil::rgb8_image_t img;
    auto png_filename = get_image("paint.png");
    image::read_png_image(png_filename, img);

    std::cout << "Read " << png_filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;

    auto view = boost::gil::view(img);
    edit_image(view);
    image::write_png_image("result.png", img);
}

void edit_and_save_jpeg() {
    // Image object used to house the frames being read
    boost::gil::rgb8_image_t img;
    auto jpeg_filename = get_image("jpeg_sonic.jpg");
    image::read_jpeg_image(jpeg_filename, img);

    std::cout << "Read " << jpeg_filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;

    auto view = boost::gil::view(img);
    edit_image(view);
    image::write_jpeg_image("result.jpg", img);
}

void print_help() {
    std::cout << "main is a demo program which reads in a PNG or JPEG image," << std::endl
              << "edits it, and writes it back to disk as result.png or result.jpg." << std::endl
              << std::endl
              << "To run the program, use:" << std::endl
              << "\tmain <filetype option> <file name>" << std::endl
              << "Where <filetype option> is one of the following values:" << std::endl
              << std::endl
              << "\t-p, --png" << std::endl
              << "\t\tIndicates the input file is a PNG image" << std::endl
              << "\t-j, --jpeg" << std::endl
              << "\t\tIndicates the input file is a JPEG image" << std::endl;
}

void print_usage() {
    std::cout << "Usage: main <filetype option> <file name>" << std::endl 
              << "Use main --help for more information." << std::endl;
}

enum input_file_type : char {
    PNG = 'p',
    JPEG = 'j', 
    UNSPECIFIED = 0
};

// Represents the parsed command-line arguments required
// to run the program
struct program_arguments {
    input_file_type file_type;
    std::vector<std::string> other_args;
};

// Parses the commandline arguments into a program_arguments
// struct. std::exit will be invoked as appropriate if 
//  (a) an invalid set of arguments is provided, or
//  (b) the help option is specified
program_arguments parse_arguments(int argc, char **argv) {
    program_arguments args;
    args.file_type = UNSPECIFIED;

    int ind = 0; // Unused but required for getopt_long
    int cur_opt; // The option currently being processed

    // This struct is defined in getopt.h. It defines all the long 
    // options that we support and their mappings to a char 
    // identifier.
    static struct option opts[] = {
        {"png",   no_argument, 0,  'p'},
        {"jpeg",  no_argument, 0,  'j'},
        {"help",  no_argument, 0,  'h'},
        {0,       0,           0,  0  }
    };

    // Defines the short versions of the options and whether they 
    // take any values
    const auto optstring = "pjh";

    // Iterate over all specified options
    while ((cur_opt = getopt_long(argc, argv, optstring, opts, &ind)) != -1) {
        switch (cur_opt) {
            case 'p':
            case 'j':
                // It is an error to specify multiple or duplicate 
                // file switches
                if (args.file_type != UNSPECIFIED) {
                    print_usage();
                    std::exit(EXIT_FAILURE);
                }
                args.file_type = static_cast<input_file_type>(cur_opt);
                break;

            case 'h':
                print_help();
                std::exit(EXIT_SUCCESS);

            case '?': // getopt failed to recognize the current option. 
            default: // getopt recognized the option, but it isn't valid.
                print_usage();
                std::exit(EXIT_FAILURE);
        }
    }

    if (args.file_type == UNSPECIFIED) {
        print_usage();
        std::exit(EXIT_FAILURE);
    }

    // Add all the non-option arguments to the result
    while (optind < argc) {
        std::string arg = argv[optind];
        args.other_args.push_back(arg);
        ++optind;
    }

    return args;
}

int main(int argc, char **argv) {
    auto args = parse_arguments(argc, argv);

    if (!args.other_args.empty()) {
        std::cout << "Extra string arguments: ";
        for (const auto& s : args.other_args) {
            std::cout << s << " ";
        }
        std::cout << std::endl;
    }

    if (args.file_type == PNG) {
        edit_and_save_png();
    }
    else {
        assert (args.file_type == JPEG);
        edit_and_save_jpeg();
    }

    return 0;
}