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

        // Creates a new color table using the provided colors in the same 
        // order as they are in the vector. The provided vector may be moved
        // from. colors' size should not exceed the maximum color table size.
        color_table(std::vector<image::rgb_pixel_t>&& colors) : table(colors) {
            assert (colors.size() <= max_size());
        }

        // Creates an empty color table.
        color_table() : table() {
        }

        // The type used to index into the table. Each index from 0
        // to size() - 1 corresponds to a pixel-color value.
        using index_type = std::uint8_t;

        // Finds the nearest color in the color table to the provided
        // pixel p and returns its index. Distance is measured as the
        // Euclidean distance between pixels in three-dimensional RGB space.
        //
        // Pre-condition: The color table must not be empty.
        index_type get_nearest_color_index(const image::rgb_pixel_t& p) const;
 
        // Adds the color contained in p to the color table.
        //
        // Pre-conditions:
        //      1. p must not already be in the color table
        //      2. size() < max_size()
        void add_color(const image::rgb_pixel_t& p);

        // Answers whether the table already contains the given
        // pixel color at any index. This is mainly used for testing/
        // assertions. It runs in O(size()) time.
        bool contains_color(const image::rgb_pixel_t& p); 

        // Gets a reference to the color at index i in O(1) time.
        const image::rgb_pixel_t& at(uint32_t i) const;

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

        std::vector<image::rgb_pixel_t> table;
    };

}

#endif