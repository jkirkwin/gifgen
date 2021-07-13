#include <cassert>
#include <iostream>
#include "args.hpp"
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
void edit_and_save_png(const std::string& filename) {
    // Image object used to house the frames being read
    boost::gil::rgb8_image_t img;
    image::read_png_image(filename, img);

    std::cout << "Read " << filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;

    auto view = boost::gil::view(img);
    edit_image(view);
    image::write_png_image("result.png", img);
}

void edit_and_save_jpeg(const std::string& filename) {
    // Image object used to house the frames being read
    boost::gil::rgb8_image_t img;
    image::read_jpeg_image(filename, img);

    std::cout << "Read " << filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;

    auto view = boost::gil::view(img);
    edit_image(view);
    image::write_jpeg_image("result.jpg", img);
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

    if (args.file_type == args::input_file_type::PNG) {
        edit_and_save_png(args.file_name);
    }
    else {
        assert (args.file_type == args::input_file_type::JPEG);
        edit_and_save_jpeg(args.file_name);
    }

    return 0;
}