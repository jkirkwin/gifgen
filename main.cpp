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

// Draw some lines in the image and write it back to disk
void edit_and_save_png(boost::gil::rgb8_image_t img, const std::string& destination) {
    auto view = boost::gil::view(img);
    edit_image(view);
    image::write_png_image(destination, img);
}

void edit_and_save_jpeg(boost::gil::rgb8_image_t img, const std::string& destination) {
    auto view = boost::gil::view(img);
    edit_image(view);
    image::write_jpeg_image(destination, img);
}

// Print the RGB pixel in the image view at the given coordinates 
template <class View>
void print_pixel(const View& view, unsigned int x, unsigned int y) {
    auto pixel = view(x, y);
    std::cout << "Pixel (" << x << ", " << y << "): " 
        << (int)pixel[0] << ", " 
        << (int)pixel[1] << ", " 
        << (int)pixel[2] << std::endl;
}

// Get a test file's relative path from its base name
std::string get_image(std::string filename) {
    const std::string test_files_dir = "../test_images/";
    return test_files_dir + filename;
}

int main() {
    // Image object used to house the frames being read
    boost::gil::rgb8_image_t img;

    // Read in a png file and print some data about it
    auto png_filename = get_image("paint.png");
    image::read_png_image(png_filename, img);
    std::cout << "Read " << png_filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;
    auto img_view = boost::gil::view(img);
    print_pixel(img_view, 0, 0);
    print_pixel(img_view, 10, 10);
    std::cout << std::endl;

    edit_and_save_png(img, "result.png");

    // Read in a jpeg file and print some data about it
    auto jpeg_filename = get_image("jpeg_sonic.jpg");
    image::read_jpeg_image(jpeg_filename, img);
    std::cout << "Read " << jpeg_filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;
    img_view = boost::gil::view(img);
    print_pixel(img_view, 0, 0);
    print_pixel(img_view, 10, 10);
    
    edit_and_save_jpeg(img, "result.jpg");

    return 0;
}
