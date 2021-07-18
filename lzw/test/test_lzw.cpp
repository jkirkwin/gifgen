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

TEST_CASE("Test flushing an empty LZW encoder", "[lzw][flush][empty]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;

    lzw_encoder<mock_buffer> encoder(4, buffer);
    encoder.flush();

    // Results should be the 5-bit clear code (10000) 
    // followed by the 5-bit EOI code (10001).
    REQUIRE(bytes.size() == 2);
    REQUIRE(bytes.at(0) == 0b00110000);
    REQUIRE(bytes.at(1) == 0b10);
}

TEST_CASE("Test encode a single literal", "[lzw][flush][literal]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;

    lzw_encoder<mock_buffer> encoder(8, buffer);
    encoder.encode(0xF1); // 0 1111 0001
    encoder.flush();

    // After encoding a single byte the result should be
    // <9 bit clear code><9 bit literal encoding><9 bit EOI code>
    // Clear code: 1 0000 0000
    // EOI code: 1 0000 0001
    REQUIRE(bytes.size() == 4);
    REQUIRE(bytes.at(0) == 0);
    REQUIRE(bytes.at(1) == 0b11100011);
    REQUIRE(bytes.at(2) == 0b00000101);
    REQUIRE(bytes.at(3) == 0b100);   
}

TEST_CASE("Test LZW encoding of short sequence", "[lzw]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;

    lzw_encoder<mock_buffer> encoder(7, buffer);

    // Input will generate patterns:
    //   130. ab
    //   131. bc
    //   132. cd
    //   133. dz
    //   134. za
    //   135. abc
    std::string input("abcdzabcd"); 

    SECTION("Use lzw_encoder::encode(char)") {
        for(auto c : input) {
            encoder.encode(c);
        }
    }
    SECTION("Use lzw_encoder::operator<<") {
        for(auto c : input) {
            encoder.encode(c);
        }
    }
    SECTION("Use lzw_encoder range encode function") {
        encoder.encode(input.begin(), input.end());
    }

    encoder.flush();

    // Results should be as follows, with 8 bit codes
    // <CC>(128), a, b, c, d, z, 130(ab), 132(cd), <EOI>(129)
    REQUIRE(encoder.code_size() == 8);
    REQUIRE(encoder.clear_code() == 128);

    REQUIRE(bytes.size() == 9);

    REQUIRE(bytes.at(0) == 128);
    REQUIRE(bytes.at(1) == 'a');
    REQUIRE(bytes.at(2) == 'b');
    REQUIRE(bytes.at(3) == 'c');
    REQUIRE(bytes.at(4) == 'd');
    REQUIRE(bytes.at(5) == 'z');
    REQUIRE(bytes.at(6) == 130);
    REQUIRE(bytes.at(7) == 132);
    REQUIRE(bytes.at(8) == 129);
}

TEST_CASE("Test LZW encoding of repeated character", "[lzw][repeated]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;

    lzw_encoder<mock_buffer> encoder(3, buffer);

    // Input will generate patterns (4 bits each):
    //   10. 44
    //   11. 444
    //   12. 4444
    unsigned char value(4);
    std::size_t repetitions(10);
    std::vector<unsigned char> input(repetitions, value);
    assert (input.size() == repetitions);

    SECTION("Use lzw_encoder::encode(char)") {
        for(auto c : input) {
            encoder.encode(c);
        }
    }
    SECTION("Use lzw_encoder::operator<<") {
        for(auto c : input) {
            encoder.encode(c);
        }
    }
    SECTION("Use lzw_encoder range encode function") {
        encoder.encode(input.begin(), input.end());
    }

    encoder.flush();

    // Results should be as follows, with 4 bit codes
    // <CC>(8), 4, 10(44), 11(444), 12(4444), <EOI>(9)
    REQUIRE(encoder.code_size() == 4);
    REQUIRE(encoder.clear_code() == 8);

    REQUIRE(bytes.size() == 3);

    REQUIRE(bytes.at(0) == 0x48); // 4, CC(0b1000)
    REQUIRE(bytes.at(1) == 0xBA); // 11, 10
    REQUIRE(bytes.at(2) == 0x9C); // EOI(9), 12 
}

TEST_CASE("Test LZW encoder code size increases", "[lzw][code size]") {
    mock_buffer buffer;
    mock_buffer::vec_type& bytes = buffer.bytes;
    lzw_encoder<mock_buffer> encoder(3, buffer);

    REQUIRE(encoder.code_size() == 4);
    REQUIRE(encoder.clear_code() == 8);
    REQUIRE(encoder.eoi_code() == 9);
    
    // With a starting code size of 4, we have 8 literals and 2 reserved 
    // codes (CC and EOI), which leaves room for 6 dictionary entries 
    // before we need to increase the code size.

    // The following input requiers 7 dictionary entries following EOI:
    //  1. #10 -- 00
    //  2. #11 -- 01
    //  3. #12 -- 11
    //  4. #13 -- 12
    //  5. #14 -- 22
    //  6. #15 -- 23
    //  7. #16 -- 33
    std::vector<unsigned char> input {
        0, 0, 1, 1, 2, 2, 3, 3
    };
    encoder.encode(input.begin(), input.end());

    // The results should be all literals (except the leading CC
    // and trailing EOI). The CC and all literals except the final 
    // 3 should be 4 bits, while the final 3 and EOI codes should be 
    // 5 bits.
    REQUIRE(encoder.code_size() == 5);
    REQUIRE(encoder.clear_code() == 8);
    REQUIRE(encoder.eoi_code() == 9);

    encoder.flush();
    REQUIRE(bytes.size() == 6);

    REQUIRE(bytes.at(0) == 0x08); // CC and first 0
    REQUIRE(bytes.at(1) == 0x10); // 2nd 0, first 1
    REQUIRE(bytes.at(2) == 0x21); // 2nd 1, first 2
    REQUIRE(bytes.at(3) == 0x32); // 2nd 2, first 3
    REQUIRE(bytes.at(4) == 0b00100011); // 2nd 3 in 5 bits, 3 bits of EOI
    REQUIRE(bytes.at(5) == 0b01); // remainder of EOI
}

TEST_CASE("Test LZW encoder clears when full", "[lzw][clear][full]") {
    FAIL("Not implemented");
    // Start with 8 bit codes and find a simple way to fill up the table
    // Could look at abc.....
    // and/or abacadaeafagahaiajakalamanaoapaqarasatauav...azbcbdbebf...bzcdcecf
}


// TODO Remove one LZW testing done. useful for debugging.
    // std::cout << "Buffer contents: ";
    // for (auto c : bytes) {
    //     std::cout << " " << (int)c;
    // }
    // std::cout << '\n';