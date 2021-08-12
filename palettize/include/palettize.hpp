#ifndef PALETTIZE_HPP
#define PALETTIZE_HPP

#include <vector>
#include "color_table.hpp"

// Provides utilities to convert a full-color (8-bit) image
// into a corresponding image representation that uses at most
// 256 colors.
//
// This process is a form of color quantization and is 
// inherantly lossy. An intelligent algorithm is used to preserve
// image quality; however, some degradation should be expected. This
// is especially apperant for images which use a large number of colors
// like gradients.
namespace palettize {

    // Creates and returns a color table of up to 256 RGB pixel 
    // colors to represent the given image as closely as possible.
    // The median cut algorithm is used to do this, with no up-front
    // scalar quantization. 
    color_table create_color_table(const image::rgb_image_view_t& image_view);

    // Quantizes image using the provided color table to produce 
    // a list of index values. Each pixel in the image is mapped
    // to a representative pixel in the color table, and the index
    // of that color table entry is added to the list. Pixel indices
    // are listed in row-major order.
    std::vector<uint8_t> palettize_image(const image::rgb_image_view_t& image_view, 
                                         const color_table& palette);
}

#endif