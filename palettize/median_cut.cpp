#include "median_cut.hpp"
#include <cassert>
#include <optional>
#include <algorithm>
#include <ranges>

namespace palettize {

    using namespace internal;

    // Packs an 8-bit RGB pixel into an integer of the form
    // 0x00RRGGBB where RR, GG, and BB are the red, green,
    // and blue values of the pixel in hex format.
    uint32_t pack_pixel(const image::rgb_pixel_t& pixel) {
        return ((pixel[0] & 0xFF) << 16)
             | ((pixel[1] & 0xFF) << 8)
             |  (pixel[2] & 0xFF);
    }

    // Compares two pixels based on all three color dimensions.
    // Red has the highest impact, followed by green, followed by
    // blue. That is, green is used to break ties where pixels'
    // red values are equal, and similarly blue is used to break 
    // red and green ties.
    auto rgb_pixel_comparator(const image::rgb_pixel_t& p1, 
                              const image::rgb_pixel_t& p2) {
        return pack_pixel(p1) < pack_pixel(p2);
    }

    // Generates a histogram of all the colors used in the image
    color_histogram 
    compute_color_histogram(const image::rgb_image_view_t& image_view) {
        color_histogram histogram;

        // Copy the data from the image into a flat vector so we can sort 
        // it to efficiently generate the histogram counts.
        std::vector<image::rgb_pixel_t> pixels(image_view.begin(), image_view.end());
        std::sort(pixels.begin(), pixels.end(), rgb_pixel_comparator);

        // Generate the histogram data by counting runs in the sorted 
        // list of pixels.
        for (const image::rgb_pixel_t& pixel : pixels) {
            if (histogram.size() > 0 && histogram.back().color == pixel) {
                // Existing run is continuing.
                ++histogram.back().count;
            }
            else {
                // A new color has been found.
                histogram.emplace_back(pixel, 1);
            }
        }

        return histogram;
    }

    // Returns a functor which compares two histogram_node's based
    // only on their color values in the given color dimension. The 
    // resulting comparator can be used to order regions in the 
    // histogram in the provided dimension. Nodes' frequency components
    // are not used.
    auto get_single_dim_histogram_comparator(color_dimension dim) {
        return [dim](const histogram_node& a, const histogram_node& b) {
            return a.color[dim] < b.color[dim];
        };
    }

    color_region::color_region(color_histogram& hist, 
                               std::size_t start, 
                               std::size_t end, 
                               uint level) : 
                histogram(hist), 
                start_index(start), 
                end_index(end), 
                level(level) {
        assert (start < end);
        assert (end <= hist.size());
        compute_bounds();
    }

    std::size_t color_region::colors() const {
        return end_index - start_index;
    }

    uint color_region::split_level() const {
        return level;
    }

    color_dimension color_region::largest_dim() const {
        assert (r_min <= r_max);
        assert (g_min <= g_max);
        assert (b_min <= b_max);

        auto r_len = r_max - r_min;
        auto g_len = g_max - g_min;
        auto b_len = b_max - b_min;

        auto max_len = std::max({r_len, g_len, b_len});
        if (r_len == max_len) {
            return color_dimension::red;
        }
        else if(g_len == max_len) {
            return color_dimension::green;
        }
        else {
            assert (b_len == max_len);
            return color_dimension::blue;
        }
    }

    bool color_region::can_split() const {
        return colors() >= 2;
    }

    color_region color_region::split_region() {
        assert (can_split());

        // Sort the region along its largest dimension.
        color_dimension dim = largest_dim();
        auto compare = get_single_dim_histogram_comparator(dim);

        auto start_it = histogram.begin() + start_index;
        auto end_it = start_it + colors();

        std::sort(start_it, end_it, compare);

        // Find the median value in the chosen dimension
        std::size_t partition_pixels(0);
        std::size_t mid_index(start_index);
        while(partition_pixels < pixel_count / 2) {
            if (mid_index == end_index - 1) {
                // In the case where all the "weight" in the region is at 
                // one end, we have to quit out early to prevent one of the
                // regions from having size 0.
                break;
            }

            partition_pixels += histogram.at(mid_index).count;
            ++mid_index;
        }

        // Split the region at the median and return the other half.
        ++level;
        color_region other(histogram, mid_index, end_index, level);
        
        end_index = mid_index;
        compute_bounds();

        return other;
    }

    void color_region::compute_bounds() {
        // Sanity check that the region is valid and non-empty.
        assert (start_index < end_index);
        assert (end_index <= histogram.size());

        r_min = 255;
        g_min = 255;
        b_min = 255;
        r_max = 0;
        g_max = 0;
        b_max = 0;
        pixel_count = 0;
        for (auto i = start_index; i < end_index; ++i) {
            auto &[color, count] = histogram.at(i);
            
            pixel_count += count;
            
            auto r = color[0];
            if (r < r_min) r_min = r;
            if (r > r_max) r_max = r;

            auto g = color[1];
            if (g < g_min) g_min = g;
            if (g > g_max) g_max = g;

            auto b = color[2];
            if (b < b_min) b_min = b;
            if (b > b_max) b_max = b;
        }
    }

    image::rgb_pixel_t color_region::average_color() const {
        std::size_t r_sum(0), g_sum(0), b_sum(0);
        for (auto i = start_index; i < end_index; ++i) {
            auto &[color, count] = histogram.at(i);
            r_sum += color[0] * count;
            g_sum += color[1] * count;
            b_sum += color[2] * count;
        }

        double double_count = pixel_count;
        double r_avg = std::round(r_sum / double_count);
        double g_avg = std::round(g_sum / double_count);
        double b_avg = std::round(b_sum / double_count);

        image::rgb_pixel_t avg;
        avg[0] = r_avg;
        avg[1] = g_avg;
        avg[2] = b_avg;

        return avg;
    }

    // Selects a region from regions with minimal level, subdivides it, 
    // and replaces it with two new regions that partition it.
    // Returns true if the subdivision was successful, or false if no
    // available region could be subdivided.
    bool subdivide_region(std::vector<color_region>& regions) {
        assert (!regions.empty());

        // Select a split-able region with the minimal level.
        auto can_split = [](const color_region& r) {
            return r.can_split();
        };
        auto splittable_regions = regions | std::views::filter(can_split);

        if (splittable_regions.empty()) {
            // None of the regions in the list can be split.
            return false;
        }

        auto compare_level = [](const color_region& r1, const color_region& r2) {
            return r1.split_level() < r2.split_level();
        };
        auto region = std::min_element(
            splittable_regions.begin(), splittable_regions.end(), compare_level
        );

        // Split the region and insert the second part of the partition into
        // the list. The first partition will already have replaced the original
        // parent region in the list.
        auto r2 = region->split_region();
        regions.push_back(r2);

        return true;
    }

    // The median cut algorithm repeatedly partitions the RGB color 
    // cube into smaller and smaller regions until a threshold is 
    // reached. Then, each region's color content is averaged to 
    // produce a representative color for that region which is added
    // to the color palette.
    // 
    // In line with other efficient implementations, a single list of 
    // colors and their frequencies is shared between all regions. A 
    // region's contents (i.e. the pixels in the image being palettized
    // whose colors belong to that region) are stored in a contiguous 
    // section of this list. The region owns that section of the list, 
    // and can sort it in-place to allow the subdivision process to use
    // O(1) extra space. 
    color_table median_cut(const image::rgb_image_view_t& image_view) {
        auto histogram = compute_color_histogram(image_view);

        // TODO Could add a log message here indicating the number of colors that were found

        if (histogram.size() <= color_table::max_size()) {
            // There are few enough colors in the image already 
            // that we can fit them all in the palette.
            color_table palette;
            for (const histogram_node& node : histogram) {
                palette.add_color(node.color);
            }
            return palette;
        }

        // To start, create a one-item list of regions containing a single region
        // representing the entire color space.
        color_region initial_region(histogram, 0, histogram.size(), 0);
        std::vector<color_region> regions { initial_region };
        regions.reserve(color_table::max_size());

        // Repeatedly choose and subdivide a region with minimal level until we
        // reach the maximum allowed number of regions.
        while(regions.size() < color_table::max_size()) {
            auto success = subdivide_region(regions);
            if (!success) {
                // No more subdivisions are possible. The image is fully palettized.
                break;
            }
        }

        // Each of the regions in the color space can be averaged to receive a 
        // representative color. These averages form the palette.
        color_table palette;
        for (const auto& region : regions) {
            palette.add_color(region.average_color());
        }

        return palette;
    }

}

