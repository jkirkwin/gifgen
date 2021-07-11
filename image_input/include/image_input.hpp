#ifndef IMAGE_INPUT_HPP
#define IMAGE_INPUT_HPP

#include <boost/gil.hpp> 

// Defines functions for reading still images from disk.
//
// The main purpose of this simple library is to hide the use of 
// the Boost::GIL IO extension from the rest of the build. This 
// is necessary because this extension (a) depends on other 
// libraries that are not otherwise needed by the project and (b)
// is not compatible with C++20.
namespace image {

    // Reads a PNG or JPEG image at the specified location into
    // the provided img object.
    //
    // Precondition: 
    //      The filename argument specifies a valid file of the 
    //      appropriate type (PNG/JPEG) with an 8-bit RGB colour
    //      encoding.
    void read_png_image(const std::string& filename, boost::gil::rgb8_image_t& img);
    void read_jpeg_image(const std::string& filename, boost::gil::rgb8_image_t& img);
}

#endif