#include <iostream>
#include "image_input.hpp"

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

    // Read in a jpeg file and print some data about it
    auto jpeg_filename = get_image("jpeg_sonic.jpg");
    image::read_jpeg_image(jpeg_filename, img);
    std::cout << "Read " << jpeg_filename << ". Dimensions: " << img.width() << "x" << img.height() << std::endl;
    img_view = boost::gil::view(img);
    print_pixel(img_view, 0, 0);
    print_pixel(img_view, 10, 10);
    
    return 0;
}
