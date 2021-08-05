#include "palettize.hpp"

namespace palettize {
    color_table create_color_table(const image::rgb_image_view_t& image_view) {

        // TODO Use median cut here

        // For the initial prototype, we generate the color table very naively.
        // We insert black and white as the first two indices, followed by the 
        // first 254 colors we find in the image.
        color_table table;
        table.add_color({0, 0, 0});
        table.add_color({0xFF, 0xFF, 0xFF});
        
        auto w = image_view.width();
        auto h = image_view.height();
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                auto pixel = image_view(x, y);

                // If the current color isn't already in the table, add it, and
                // quit the loop if we've filled the color table all the way.
                if (!table.contains_color(pixel)) {
                    table.add_color(pixel);

                    if (table.size() == color_table::max_size()) {
                        return table;
                    }
                }
            }
        }

        return table;

    } // TODO test

    std::vector<uint8_t> palettize_image(const image::rgb_image_t& image_view, const color_table& palette) {
        assert ((void*)&image_view != (void*)&palette);
        throw std::runtime_error("palettize_image isn't implemented");
    } // TODO Implement and test

}

