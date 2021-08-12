#ifndef MEDIAN_CUT_HPP
#define MEDIAN_CUT_HPP

#include "image_utils.hpp"
#include "color_table.hpp"

namespace palettize {


    // Holds implementation details which are not part of the public interface
    namespace internal {

        // Enumerates the color space dimensions available in the median
        // cut search space.
        enum color_dimension {
            red, green, blue
        };

        // A histogram entry which records the number of times a color
        // ocurrs in an image.
        struct histogram_node {
            image::rgb_pixel_t color;
            std::size_t count;
        };

        // A histogram showing the number of occurrences of each color
        // present in an image.
        using color_histogram = std::vector<histogram_node>;

        // Represents a non-empty 3-dimensional region in the 0-255 RGB cube 
        // and the set of colors in that region that are used in an image.
        // The region's color data is stored in a contiguous portion of the 
        // shared color histogram.
        class color_region {
        public:

            // Creates a new color_region which spans the range [start, end) in 
            // color_list. lim is the initial level of the region. This should be one
            // more than its parent region.
            color_region(color_histogram& histogram, 
                         std::size_t start,
                         std::size_t end,
                         uint level);
            
            ~color_region() = default;

            color_region(const color_region&) = default;
            color_region& operator=(const color_region&) = default;

            color_region(color_region&&) = default;
            color_region& operator=(color_region&&) = default;

            // Returns the number of unique colors in the region.
            std::size_t colors() const;

            // Returns the number of times the parent region was split
            // before this one was created.
            uint split_level() const;

            // Partitions the region into two sub-regions along this region's
            // largest dimension. The partition is taken about the median color
            // point. The sub-region "below" the median is stored in this object.
            // The sub-regio "above" the median is returned.
            //
            // The region must contain at least two colors with distinct color 
            // values.
            color_region split_region();

            // Answers whether this region contains sufficient data to be 
            // subdivided in two.
            bool can_split() const;

            // Returns a pixel value representing the average color in this
            // region of the image. The average is weighted over all pixels
            // for more representative results.
            image::rgb_pixel_t average_color() const;

        private: 
            // The shared list and markers showing where this region lies in
            // the list. This region owns this section of the list and may
            // re-order it as necessary. We use raw indices rather than 
            // iterators due to some of the lookahead checks that are required.
            color_histogram& histogram; 
            std::size_t start_index;
            std::size_t end_index;

            // The split-level. Used to choose regions for splitting.
            uint level;

            // The number of pixels from the image that are inside this region.
            // Equal to the sum over the counts of all colors in the region.
            std::size_t pixel_count;

            // Dimensions/position of the color region.
            uint r_min, r_max;
            uint g_min, g_max;
            uint b_min, b_max;

            // Updates the boundary information using the allocated section of 
            // the shared color list. Used at construction and after splitting.
            void compute_bounds();

            // Returns the largest dimension in the region. Ties are broken 
            // arbitrarily.
            color_dimension largest_dim() const;
        };
    }


    // Uses the Median Cut color quantization algorithm to produce a
    // color palette suitable for GIF-encoding. The algorithm is 
    // implemented classically, with the exception that the initial
    // scalar quantization step is omitted to produce higher fidelity
    // results. The implementation follows the layout suggested by 
    // Burger and Burge in Principals of Digital Image Processing,
    // but handles some missed edge cases in their treatment of it,
    // and uses less space.
    color_table median_cut(const image::rgb_image_view_t& image_view);

}

#endif