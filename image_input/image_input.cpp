#include "image_input.hpp"
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/extension/io/jpeg.hpp>

namespace image {
    void read_png_image(const std::string& filename, boost::gil::rgb8_image_t& img) {
        boost::gil::read_image(filename, img, boost::gil::png_tag{});
    }

    void read_jpeg_image(const std::string& filename, boost::gil::rgb8_image_t& img) {
        boost::gil::read_image(filename, img, boost::gil::jpeg_tag{});
    }
}
