#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <sstream>
#include "sub_block_buffer.hpp"

using namespace gif;

TEST_CASE("Test manually write empty block") {
    std::stringstream ss;
    sub_block_buffer buffer(ss);
    REQUIRE(ss.str().empty());
    REQUIRE(buffer.current_block_size() == 0);

    buffer.write_current_block();
    REQUIRE(buffer.current_block_size() == 0);
    REQUIRE(ss.str().size() == 1);
    REQUIRE(ss.str().at(0) == char(0));
}

TEST_CASE("Test manually write partial block") {
    std::stringstream ss;
    sub_block_buffer buffer(ss);
    REQUIRE(ss.str().empty());
    REQUIRE(buffer.current_block_size() == 0);

    buffer << 'h' << 'e' << 'l' << 'l' << 'o';
    REQUIRE(buffer.current_block_size() == 5);
    REQUIRE(ss.str().empty());

    buffer.write_current_block();
    REQUIRE(buffer.current_block_size() == 0);
    REQUIRE(ss.str().size() == 6);
    REQUIRE(ss.str().at(0) == 5);
    REQUIRE(ss.str().substr(1) == "hello");
}

TEST_CASE("Test buffer writes when filled") {
    std::stringstream ss;
    sub_block_buffer buffer(ss);

    for (std::size_t i = 0; i < 255; ++i) {
        REQUIRE(ss.str().size() == 0);
        REQUIRE(buffer.current_block_size() == i);
        buffer << static_cast<char>(i);
    }

    REQUIRE(ss.str().size() == MAX_IMAGE_SUB_BLOCK_SIZE + 1);
    REQUIRE(buffer.current_block_size() == 0);

    REQUIRE(ss.str().at(0) == char(0xFF));

    std::string block_data = ss.str().substr(1);
    REQUIRE(block_data.size() == MAX_IMAGE_SUB_BLOCK_SIZE);
    for (std::size_t i = 0; i < block_data.size(); ++i) {
        REQUIRE(block_data.at(i) == char(i));
    }
}
