#include "palettize.hpp"
#include "median_cut.hpp"

namespace palettize {

    color_table create_color_table(const image::rgb_image_view_t& image_view) {
        return median_cut(image_view);
    }

    std::vector<uint8_t> palettize_image(const image::rgb_image_view_t& image_view, const color_table& palette) {
        auto w = image_view.width();
        auto h = image_view.height();

        std::vector<uint8_t> indices;
        indices.reserve(w * h);
        
        for (auto& pixel : image_view) {
            auto index = palette.get_nearest_color_index(pixel);
            indices.push_back(index);
        }

        return indices;
    }

}

