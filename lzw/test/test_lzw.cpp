#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "lzw.hpp"

using namespace lzw;

// Used as the downstream target for LZW-encoded data.
struct byte_buffer {
    using vec_type = std::vector<uint8_t>;

    vec_type bytes;

    byte_buffer& operator<<(uint8_t byte) {
        bytes.push_back(byte);
        return *this;
    }
};

TEST_CASE("Test LZW encoder initial state", "[lzw][lifecycle]") {
    byte_buffer buffer;
    byte_buffer::vec_type& bytes = buffer.bytes;

    SECTION("Min size = 3") {
        lzw_encoder<byte_buffer> encoder(3, buffer);
        CHECK(encoder.code_size() == 4);
        CHECK(encoder.clear_code() == 8); // 0b1000
        CHECK(encoder.eoi_code() == 9);
        REQUIRE(bytes.size() == 0);
    }
    SECTION("Min size = 4") {
        lzw_encoder<byte_buffer> encoder(4, buffer);
        CHECK(encoder.code_size() == 5);
        CHECK(encoder.clear_code() == 16);
        CHECK(encoder.eoi_code() == 17);
        REQUIRE(bytes.size() == 0);
    }
    SECTION("Min size = 8") {
        lzw_encoder<byte_buffer> encoder(8, buffer);
        CHECK(encoder.code_size() == 9);
        CHECK(encoder.clear_code() == 256);
        CHECK(encoder.eoi_code() == 257);
        // We don't check that the size is 0 here because the 
        // encoder might have written the initial clear code
        // downstream already since it takes more than one byte.
    }
}

TEST_CASE("Test construct const LZW encoder", "[lzw][lifecycle]") {
    byte_buffer buffer;
    byte_buffer::vec_type& bytes = buffer.bytes;

    const lzw_encoder<byte_buffer> encoder(5, buffer);
    CHECK(encoder.code_size() == 6);
    CHECK(encoder.clear_code() == 0b100000);
    CHECK(encoder.eoi_code() == 0b100001);
    REQUIRE(bytes.size() == 0);
}

// TODO Add some tests explicitly for the byte-buffering logic in
// internal::byte_buffer
