#ifndef COMMON_IMAGE_UTILS
#define COMMON_IMAGE_UTILS

#include <boost/gil.hpp>

// This file contains type definitions and utilities to improve
// the readability of image processing code that relies on 
// boost::gil.
namespace image {
    using rgb_pixel_t = boost::gil::rgb8_pixel_t;
    using rgb_image_t = boost::gil::rgb8_image_t;
    using rgb_image_view_t = boost::gil::rgb8_view_t;
}

#endif
