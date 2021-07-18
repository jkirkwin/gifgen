#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "lzw.hpp"

using namespace lzw;

// Used as the downstream target for LZW-encoded data.
struct mock_buffer {
    using vec_type = std::vector<uint8_t>;

    vec_type bytes;

    mock_buffer& operator<<(uint8_t byte) {
        bytes.push_back(byte);
        return *this;
    }
};

TEST_CASE("Test LZW encoder initial state", "[lzw][lifecycle]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;

    SECTION("Min size = 3") {
        lzw_encoder<mock_buffer> encoder(3, buffer);
        CHECK(encoder.code_size() == 4);
        CHECK(encoder.clear_code() == 8); // 0b1000
        CHECK(encoder.eoi_code() == 9);
        REQUIRE(bytes.size() == 0);
    }
    SECTION("Min size = 4") {
        lzw_encoder<mock_buffer> encoder(4, buffer);
        CHECK(encoder.code_size() == 5);
        CHECK(encoder.clear_code() == 16);
        CHECK(encoder.eoi_code() == 17);
        REQUIRE(bytes.size() == 0);
    }
    SECTION("Min size = 8") {
        lzw_encoder<mock_buffer> encoder(8, buffer);
        CHECK(encoder.code_size() == 9);
        CHECK(encoder.clear_code() == 256);
        CHECK(encoder.eoi_code() == 257);
        // We don't check that the size is 0 here because the 
        // encoder might have written the initial clear code
        // downstream already since it takes more than one byte.
    }
}

TEST_CASE("Test construct const LZW encoder", "[lzw][lifecycle]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;

    const lzw_encoder<mock_buffer> encoder(5, buffer);
    CHECK(encoder.code_size() == 6);
    CHECK(encoder.clear_code() == 0b100000);
    CHECK(encoder.eoi_code() == 0b100001);
    REQUIRE(bytes.size() == 0);
}

TEST_CASE("Test flush empty byte buffer", "[lzw][buffer]") {
    // Make a mock downstream buffer
    mock_buffer downstream_buf;
    mock_buffer::vec_type& downstream_bytes = downstream_buf.bytes;

    // Create the byte buffer
    internal::byte_buffer<mock_buffer> buffer(downstream_buf);
    REQUIRE(downstream_bytes.empty());

    // Flushing an empty buffer should not cause anything to be
    // written downstream.
    buffer.flush();
    REQUIRE(downstream_bytes.empty());
}

TEST_CASE("Test flush partial byte from byte buffer", "[lzw][buffer]") {
    // Make a mock downstream buffer
    mock_buffer downstream_buf;
    mock_buffer::vec_type& downstream_bytes = downstream_buf.bytes;

    // Create the byte buffer
    internal::byte_buffer<mock_buffer> buffer(downstream_buf);
    REQUIRE(downstream_bytes.empty());

    // Inserting less than 8 bits should not cause anything to be
    // written downstream.
    buffer.insert(6, 0xFF); // 111111
    REQUIRE(downstream_bytes.empty());

    // Flushing the buffer should write a partial byte downstream.
    buffer.flush();
    REQUIRE(downstream_bytes.size() == 1);
    REQUIRE(downstream_bytes.at(0) == 0b111111);
}

TEST_CASE("Test byte buffer output ordering", "[lzw][buffer]") {
    // Make a mock downstream buffer
    mock_buffer downstream_buf;
    mock_buffer::vec_type& downstream_bytes = downstream_buf.bytes;

    // Create the byte buffer
    internal::byte_buffer<mock_buffer> buffer(downstream_buf);
    REQUIRE(downstream_bytes.empty());

    // Insert a handful of codes of different lengths.
    // This should result in at least some of the data
    // being written downstream. Flushing should yield
    // the rest of the data.
    buffer.insert(5, 0xFF); // 0b11111
    buffer.insert(3, 1); // 0b001
    buffer.insert(2, 2); // 0b10
    buffer.insert(7, 7); // 0b0000111
    buffer.insert(1, 1); // 0b1

    REQUIRE(!downstream_bytes.empty());

    buffer.flush();
    REQUIRE(downstream_bytes.size() == 3);
    REQUIRE(downstream_bytes.at(0) == 0b00111111); // First and second insertions
    REQUIRE(downstream_bytes.at(1) == 0b00011110); // Third and part of fourth insertions
    REQUIRE(downstream_bytes.at(2) == 0b10); // MSBit of fourth insertion and fifth
}

TEST_CASE("Test writing large code word to byte buffer", "[lzw][buffer]") {
    // Make a mock downstream buffer
    mock_buffer downstream_buf;
    mock_buffer::vec_type& downstream_bytes = downstream_buf.bytes;

    // Create the byte buffer
    internal::byte_buffer<mock_buffer> buffer(downstream_buf);

    // Insert more than 8 bits a few times
    buffer.insert(9, 0x10F); // 1 0000 1111
    buffer.insert(15, 0x380F); // 011 1000 0000 1111
    REQUIRE(!downstream_bytes.empty());

    buffer.flush();
    // Results should be: 0000 1111 0001 1111 0111 0000
    REQUIRE(downstream_bytes.size() == 3);
    REQUIRE(downstream_bytes.at(0) == 0x0F); 
    REQUIRE(downstream_bytes.at(1) == 0x1F);
    REQUIRE(downstream_bytes.at(2) == 0x70); 
}

TEST_CASE("Test writing whole bytes to a byte buffer", "[lzw][buffer]") {
    // Make a mock downstream buffer
    mock_buffer downstream_buf;
    mock_buffer::vec_type& downstream_bytes = downstream_buf.bytes;

    // Create the byte buffer
    internal::byte_buffer<mock_buffer> buffer(downstream_buf);

    for (unsigned char uc = 0; uc < 129; ++uc) {
        buffer.insert(8, uc);
    } 
    buffer.flush();

    REQUIRE(downstream_bytes.size() == 129);
    for (unsigned char uc = 0; uc < 129; ++uc) {
        REQUIRE(downstream_bytes.at(uc) == uc);
    } 
}
