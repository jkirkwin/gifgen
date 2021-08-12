#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "color_table.hpp"

using namespace palettize;

// A simple factory that produces a sequence of unique 
// (non-repeating) colors.
class unique_color_factory {
public:
    // Create a new color factory. The first value will be pure 
    // black.
    unique_color_factory() : rgb(0) {}

    // Create a new color factory which starts generating colors
    // using the given seed.
    unique_color_factory(uint32_t seed) : rgb(seed) {}

    image::rgb_pixel_t operator()() {
        image::rgb_pixel_t color(r(), g(), b());
        ++rgb;
        return color;
    }

private:
    // A packed RGB representation.
    uint32_t rgb;

    uint8_t r() {
        return (rgb & 0xFF0000) >> 16;
    }

    uint8_t g() {
        return (rgb & 0xFF00) >> 8;
    }

    uint8_t b() {
        return rgb & 0xFF;
    }
};

image::rgb_pixel_t get_random_color() {
    return image::rgb_pixel_t(
        rand() % 0xFF,
        rand() % 0xFF,
        rand() % 0xFF
    );
}

TEST_CASE("Test add colors to color table", "[color_table]") {
    color_table palette;
    
    for (uint i = 0; i <= 0xFF; ++i) {
        REQUIRE(palette.size() == i);
        image::rgb_pixel_t color(i, i, i);
        palette.add_color(color);
        REQUIRE(palette.at(i) == color);
    }
    
    REQUIRE(palette.size() == color_table::max_size());
    REQUIRE(palette.contains_color(image::rgb_pixel_t(10, 10, 10)));
}

TEST_CASE("Test bit-depth reported by color table", "[color_table][bit_depth]") {
    color_table palette;
    unique_color_factory color_factory(rand());

    REQUIRE(palette.size() == 0);
    CHECK(palette.min_bit_depth() == 0);

    palette.add_color(color_factory());
    REQUIRE(palette.size() == 1);
    CHECK(palette.min_bit_depth() == 1);

    palette.add_color(color_factory());
    REQUIRE(palette.size() == 2);
    CHECK(palette.min_bit_depth() == 1);

    palette.add_color(color_factory());
    REQUIRE(palette.size() == 3);
    CHECK(palette.min_bit_depth() == 2);

    palette.add_color(color_factory());
    REQUIRE(palette.size() == 4);
    CHECK(palette.min_bit_depth() == 2);

    palette.add_color(color_factory());
    REQUIRE(palette.size() == 5);
    CHECK(palette.min_bit_depth() == 3);

    palette.add_color(color_factory());
    palette.add_color(color_factory());
    palette.add_color(color_factory());
    REQUIRE(palette.size() == 8);
    CHECK(palette.min_bit_depth() == 3);

    palette.add_color(color_factory());
    REQUIRE(palette.size() == 9);
    CHECK(palette.min_bit_depth() == 4);


    while (palette.size() < 128) {
        palette.add_color(color_factory());
    }
    REQUIRE(palette.size() == 128);
    REQUIRE(palette.min_bit_depth() == 7);
} 

TEST_CASE("Test get nearest color from color table with size 1" , "[color_table][nearest]") {
    color_table palette;
    image::rgb_pixel_t table_color = get_random_color();
    palette.add_color(table_color);

    // Check that for any input, the nearest color is the table color
    for (int i = 0; i < 25; ++i) {
        auto test_color = get_random_color();
        REQUIRE(palette.get_nearest_color_index(test_color) == 0);
    }
}

TEST_CASE("Test get nearest color from color table with size > 1" , "[color_table][nearest]") {
    color_table palette;
    image::rgb_pixel_t white(0xFF, 0xFF, 0xFF);
    image::rgb_pixel_t grey(0x80, 0x80, 0x80);
    image::rgb_pixel_t black(0, 0, 0);
    palette.add_color(white);
    palette.add_color(grey);
    palette.add_color(black);

    color_table::index_type white_index = 0;
    color_table::index_type grey_index = 1;
    color_table::index_type black_index = 2;

    REQUIRE(palette.at(white_index) == white);
    REQUIRE(palette.at(grey_index) == grey);
    REQUIRE(palette.at(black_index) == black);

    REQUIRE(palette.get_nearest_color_index(white) == white_index);
    REQUIRE(palette.get_nearest_color_index(grey) == grey_index);
    REQUIRE(palette.get_nearest_color_index(black) == black_index);

    image::rgb_pixel_t close_to_white(200, 199, 201);
    REQUIRE(palette.get_nearest_color_index(close_to_white) == white_index);

    image::rgb_pixel_t close_to_grey(120, 120, 10);
    REQUIRE(palette.get_nearest_color_index(close_to_grey) == grey_index);

    image::rgb_pixel_t close_to_black(40, 40, 50);
    REQUIRE(palette.get_nearest_color_index(close_to_black) == black_index);
}
