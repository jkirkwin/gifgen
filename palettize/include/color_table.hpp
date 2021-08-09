#ifndef COLOR_TABLE_HPP
#define COLOR_TABLE_HPP

#include "image_utils.hpp"
#include <boost/gil.hpp>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <utility>

namespace palettize {

    // Represents a GIF color table which acts as a palette
    // of colors with which to quantize and encode one or more
    // frames of image data.
    //
    // A color table may support up to 256 distinct colors.
    class color_table {
    public:

        // The type used to index into the table. Each index from 0
        // to size() - 1 corresponds to a pixel-color value.
        using index_type = std::uint8_t;

        // Finds the nearest color in the color table to the provided
        // pixel p. Distance is measured as the Euclidean distance 
        // between pixels in three-dimensional RGB space.
        //
        // Returns the nearest color and the index in the table 
        // corresponding to the color.
        //
        // Pre-condition: The table must not be empty.
        std::pair<index_type, image::rgb_pixel_t> get_nearest_color(const image::rgb_pixel_t& p) const;
 
        // Adds the color contained in p to the color table.
        //
        // Pre-conditions:
        //      1. p must not already be in the color table
        //      2. size() < max_size()
        void add_color(const image::rgb_pixel_t& p);

        // Answers whether the table already contains the given
        // pixel color at any index.
        bool contains_color(const image::rgb_pixel_t& p); // TODO remove or optimize once median cut is implemented.

        // Gets a reference to the color at index i
        const image::rgb_pixel_t& at(uint32_t i) const;

        // TODO Add iterators so we can easily iterate over the color table to write it to the output file

        // Returns the number of bits needed to index into the table. 
        uint8_t min_bit_depth() const;

        // Returns the number of entries in the color table.
        std::size_t size() const;

        // Returns the maximum size that a color table is 
        // permitted to reach.
        static std::size_t max_size();

    private:       
        // The maximum distance for RGB values capped at 255 is 
        //      (255-0)^2 + (255-0)^2 + (255-0)^2 
        //    = 3 * 255^3  
        //    = 195075
        static const uint32_t MAX_EUCLIDEAN_DISTANCE = 195075;

        std::vector<image::rgb_pixel_t> table; // TODO Replace with whatever type of tree the median cut algorithm generate
    };

}

#endif