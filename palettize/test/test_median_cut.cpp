// Since (in)correctness of the code is more obvious than normal
// for this part of the application due to its visual nature, the
// internal components are tested more thoroughly than the high-level
// logic of the median cut algorithm.

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "boost/gil.hpp"
#include "image_utils.hpp"
#include "median_cut.hpp"

using namespace palettize;
using namespace internal;

TEST_CASE("Test unit color region average color computation", "[median_cut][region][avg]") {
    image::rgb_pixel_t color(0x12, 0x98, 0x21);
    color_freq node(color, 1);
    color_region::color_freq_list list { node };

    color_region region(list, 0, 1, 0);

    REQUIRE(region.colors() == 1);
    REQUIRE(region.split_level() == 0);
    REQUIRE(region.average_color() == color);
}

TEST_CASE("Test color region average color computation", "[median_cut][region][avg]") {
    image::rgb_pixel_t c1(255, 1, 100);
    image::rgb_pixel_t c2(0, 0, 0);
    image::rgb_pixel_t c3(50, 40, 30);
    image::rgb_pixel_t c4(100, 1, 9);
    color_region::color_freq_list list {
        color_freq(c1, 6),
        color_freq(c2, 1),
        color_freq(c3, 2),
        color_freq(c4, 3)
    };
    color_region region(list, 0, 4, 0);

    image::rgb_pixel_t expected_avg(161, 7, 57); // Computed by hand

    REQUIRE(region.colors() == 4);
    REQUIRE(region.average_color() == expected_avg);
}

TEST_CASE("Test split region with two colors", "[region][split][median_cut]") {
    // Create two colors which give red the largest variance
    image::rgb_pixel_t color_1(255, 100, 50);
    image::rgb_pixel_t color_2(0, 80, 60);

    // Use a variety of weightings
    std::size_t count_1, count_2;
    SECTION("Even weighting") {
        count_1 = 1;
        count_2 = 1;
    }
    SECTION("High-red weighting") {
        count_1 = 10; 
        count_2 = 1;
    }
    SECTION("low-red weighting") {
        count_1 = 1;
        count_2 = 10; 
    }

    color_region::color_freq_list color_list { 
        color_freq(color_1, count_1),
        color_freq(color_2, count_2)
    };
    
    color_region region_1(color_list, 0, 2, 0);
    REQUIRE(region_1.split_level() == 0);
    REQUIRE(region_1.colors() == 2);
    
    auto region_2 = region_1.split_region();
    REQUIRE(region_1.split_level() == 1);
    REQUIRE(region_1.colors() == 1);
    REQUIRE(region_1.average_color() == color_2); // Color two has less red

    REQUIRE(region_2.split_level() == 1);
    REQUIRE(region_2.colors() == 1);
    REQUIRE(region_2.average_color() == color_1); // Color one has more red
}

TEST_CASE("Test split region on green dimension", "[region][split][median_cut]") {
    // Create two colors which give green the largest variance.
    // If the split is done on either of the other dimensions, 
    // the order will be reversed.
    image::rgb_pixel_t color_1(99, 80, 74);
    image::rgb_pixel_t color_2(0, 180, 50);

    color_region::color_freq_list color_list { 
        color_freq(color_1, 1),
        color_freq(color_2, 1)
    };

    color_region region_1(color_list, 0, 2, 10);
    auto region_2 = region_1.split_region();

    REQUIRE(region_1.colors() == 1);
    REQUIRE(region_2.colors() == 1);

    REQUIRE(region_1.split_level() == 11);
    REQUIRE(region_2.split_level() == 11);

    REQUIRE(region_1.average_color() == color_1);
    REQUIRE(region_2.average_color() == color_2);
}

TEST_CASE("Test split region on blue dimension", "[region][split][median_cut]") {
    // Create two colors which give blue the largest variance.
    // If the split is done on either of the other dimensions, 
    // the order will be reversed.
    image::rgb_pixel_t color_1(99, 255, 100);
    image::rgb_pixel_t color_2(0, 180, 200);

    color_region::color_freq_list color_list { 
        color_freq(color_1, 1),
        color_freq(color_2, 1)
    };

    color_region region_1(color_list, 0, 2, 10);
    auto region_2 = region_1.split_region();

    REQUIRE(region_1.colors() == 1);
    REQUIRE(region_2.colors() == 1);

    REQUIRE(region_1.split_level() == 11);
    REQUIRE(region_2.split_level() == 11);

    REQUIRE(region_1.average_color() == color_1);
    REQUIRE(region_2.average_color() == color_2);
}

TEST_CASE("Smoke test median cut for image with one pixel", "[smoke][1x1][median_cut]") {
    image::rgb_image_t img(1, 1);
    image::rgb_image_view_t img_view = view(img);
    image::rgb_pixel_t color(0x11, 0x22, 0x33);
    img_view(0, 0) = color;

    auto palette = median_cut(img_view);
    REQUIRE(palette.size() == 1);
    REQUIRE(palette.contains_color(color));
}

TEST_CASE("Smoke test median cut for monochrome image", "[smoke][monochrome][median_cut]") {
    std::size_t w = 100, h = 50;
    image::rgb_image_t img(w, h);
    image::rgb_image_view_t img_view = view(img);

    image::rgb_pixel_t color(0xF0, 0x0F, 0x66);
    for (auto& pixel : img_view) pixel = color;

    auto palette = median_cut(img_view);
    
    REQUIRE(palette.size() == 1);
    REQUIRE(palette.contains_color(color));
}

TEST_CASE("Test median cut yields full color table for image with 256 unique colors", "[median_cut]") {
    std::size_t w = 10, h = 256;
    image::rgb_image_t img(w, h);
    image::rgb_image_view_t img_view = view(img);

    // Color each row of the image with a different color.
    // Every pixel will have the same blue and green values, and each row will have a 
    // unique red value.
    uint8_t green(0x55);
    uint8_t blue(0x01);
    for (std::size_t y = 0; y < h; ++y) {
        image::rgb_pixel_t row_color(y, green, blue);
        for (std::size_t x = 0; x < w; ++x) {
            img_view(x, y) = row_color;
        }
    }

    auto palette = median_cut(img_view);

    REQUIRE(palette.size() == 256);
    REQUIRE(palette.size() == color_table::max_size());

    for (std::size_t y = 0; y < h; ++y) {
        image::rgb_pixel_t row_color(y, green, blue);
        REQUIRE(palette.contains_color(row_color));
    }
}

TEST_CASE("Smoke test median cut on image with more than 256 colors", "[median_cut][smoke]") {
    // Create an image with 257 colors and verify that one pair of colors has changed.

    std::size_t w = 10, h = 257;
    image::rgb_image_t img(w, h);
    image::rgb_image_view_t img_view = view(img);

    uint8_t green(0xF0), blue(0x0F);

    std::vector<image::rgb_pixel_t> colors;

    // Fill the first 256 rows with a red gradient
    for (std::size_t y = 0; y < 256; ++y) {
        image::rgb_pixel_t row_color(y, green, blue);
        colors.push_back(row_color);
        for (std::size_t x = 0; x < w; ++x) {
            img_view(x, y) = row_color;
        }
    }

    // Fill the last row with a different color
    image::rgb_pixel_t last_color(0x01, green - 10, blue + 10);
    colors.push_back(last_color); 
    for (std::size_t x = 0; x < w; ++x) {
        img_view(x, 256) = last_color;
    }

    auto palette = median_cut(img_view);
    REQUIRE(palette.size() == color_table::max_size());

    uint missing_count(0);
    for (const auto& color : colors) {
        if (!palette.contains_color(color)) {
            ++missing_count;
        }
    }

    // Since we used 257 colors in total, there may be either one or
    // two colors which don't appear exactly in the output. Whether the
    // count is one or two depends on how the averaging works out, and 
    // isn't important for this test.
    REQUIRE(missing_count > 0);
    REQUIRE(missing_count <= 2);
}
