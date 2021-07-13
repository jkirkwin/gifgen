#ifndef IMAGE_IO_HPP
#define IMAGE_IO_HPP

#include <boost/gil.hpp> 

// Defines functions for reading and writing still images from 
// and to disk.
//
// The main purpose of this simple library is to hide the use of 
// the Boost::GIL IO extension from the rest of the build. This 
// is necessary because this extension (a) depends on other 
// libraries that are not otherwise needed by the project and (b)
// is not compatible with C++20, so this allows the rest of the 
// codebase to use this functionality *and* be compiled with C++20.
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

    // Writes a PNG or JPEG image to a file at the provided path.
    // If no such file exists, a new one will be created. If the
    // file path identifies an existing file, it will be overwritten. 
    void write_png_image(const std::string& filename, const boost::gil::rgb8_image_t& img);
    void write_jpeg_image(const std::string& filename, const boost::gil::rgb8_image_t& img);
}

#endif