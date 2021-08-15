#include "image_io.hpp"
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/extension/io/jpeg.hpp>

namespace image {

    // Due to how boost::gil handles different file types, image layouts,
    // etc., there isn't a clean generic way to work specifically with JPEG
    // and PNG files as we want to without a little duplication. The jpeg and 
    // png "tags" that are needed to read and write those formats do not have 
    // a common type, which restricts how they may be used. This is the reason
    // for the delegation approach taken here, which does unfortunately introduce
    // some duplication, but allows the code to remain dead-simple. 

    template<class BoostFileTag>
    bool boost_is_file_type(const std::string& filename) {
        // Unfortunately, GIL doesn't give us a nice way to query an image file's
        // encoding, so we try to read the image header as the given file type, 
        // and return false if this fails.
        try {
            boost::gil::read_image_info(filename, BoostFileTag{});
            return true;
        }
        catch(...) {
            return false;
        }
    }

    bool is_file_type(const std::string& filename, file_type type) {
        assert (type == file_type::JPEG || type == file_type::PNG);
        return type == file_type::JPEG 
            ? boost_is_file_type<boost::gil::jpeg_tag>(filename) 
            : boost_is_file_type<boost::gil::png_tag>(filename);
    }

    void read_image(const std::string& filename, boost::gil::rgb8_image_t& img, file_type type) {
        assert (type == file_type::JPEG || type == file_type::PNG);
        return type == file_type::JPEG 
            ? read_jpeg_image(filename, img) 
            : read_png_image(filename, img); 
    }

    void read_png_image(const std::string& filename, boost::gil::rgb8_image_t& img) {
        boost::gil::read_image(filename, img, boost::gil::png_tag{});
    }

    void read_jpeg_image(const std::string& filename, boost::gil::rgb8_image_t& img) {
        boost::gil::read_image(filename, img, boost::gil::jpeg_tag{});
    }

    void write_image(const std::string& filename, boost::gil::rgb8_image_t& img, file_type type) {
        assert (type == file_type::JPEG || type == file_type::PNG);
        return type == file_type::JPEG 
            ? write_jpeg_image(filename, img) 
            : write_png_image(filename, img); 
    }

    void write_png_image(const std::string& filename, const boost::gil::rgb8_image_t& img) {
        boost::gil::write_view(filename, boost::gil::const_view(img), boost::gil::png_tag{});
    }

    void write_jpeg_image(const std::string& filename, const boost::gil::rgb8_image_t& img) {
        boost::gil::write_view(filename, boost::gil::const_view(img), boost::gil::jpeg_tag{});
    }
}
