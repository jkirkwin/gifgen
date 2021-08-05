#include "color_table.hpp"

namespace palettize {
    
    // Computes the squared Euclidean distance between the points 
    // p1 and p2 in 3-dimensional RGB space. The formula for this
    // quantity is
    //      (r2 - r1)^2 + (g2 - g1)^2 + (b2 - b1)^2
    // where pixel pi = (ri, gi, bi) 
    uint32_t euclidean_distance(const image::rgb_pixel_t& p1, const image::rgb_pixel_t& p2) {
        uint32_t distance = 0;
        for (int i = 0; i < 3; ++i) {
            // Since we may be dealing with unsigned types, compute the 
            // absolute difference my subtracting min from max to prevent
            // overflow. 
            auto difference = std::max(p1[i], p2[i]) - std::min(p1[i], p2[i]);
            distance += difference * difference;
        }
        return distance;
    }

    std::pair<color_table::index_type, image::rgb_pixel_t> 
    color_table::get_nearest_color(const image::rgb_pixel_t& p) const {
        assert (size() > 0);

        uint32_t min_distance = MAX_EUCLIDEAN_DISTANCE + 1;
        index_type index = 0;
        for (std::size_t i = 0; i < size(); ++i) {
            uint32_t new_distance = euclidean_distance(p, table.at(i));
            if (new_distance < min_distance) {
                min_distance = new_distance;
                index = static_cast<index_type>(i);

                // If we have a direct match, return immediately.
                if (min_distance == 0) {
                    break;
                }
            }              
        }

        return std::make_pair(index, table[index]);
    } 
    // TODO test

    void color_table::add_color(const image::rgb_pixel_t& p) {
        assert (size() < max_size());
        table.push_back(p);
    }
    // TODO test

    bool color_table::contains_color(const image::rgb_pixel_t& p) {  // TODO remove or optimize once median cut is implemented.
        for (const auto& entry : table) {
            if (entry == p) {
                return true;
            }
        }
        return false;
    }

    // Returns the number of entries in the color table.
    std::size_t color_table::size() const {
        return table.size();
    } 
    // TODO test

    std::size_t color_table::max_size() {
        return 256;
    }

    // TODO I think the Median cut implementation will give us
    //      an intelligent structure we can traverse nicely, so 
    //      for now we will just use a hash map for now.
}
