#ifndef GIF_DATA_FORMAT_HPP
#define GIF_DATA_FORMAT_HPP

#include <cassert>
#include <cstdint>

namespace gif {
    constexpr uint8_t LZW_CODE_SIZE = 8;
    
    constexpr std::size_t MAX_IMAGE_SUB_BLOCK_SIZE = 255;
    constexpr std::size_t SCREEN_DESCRIPTOR_SIZE = 7;
    constexpr std::size_t IMAGE_DESCRIPTOR_SIZE = 10;

    constexpr uint8_t IMAGE_SEPARATOR_BYTE = 0x2C;
    constexpr uint8_t GIF_TRAILER_BYTE = 0x3B;

    // See the GIF specification for details on the composition
    // of this packed byte. We set the global color table flag
    // to 0, the color resolution to 8 (encoded as 7), the sort
    // flag to 0, and the global color table size to all zeros.
    constexpr std::size_t SCREEN_DESCRIPTOR_PACKED_BYTE = 0x70;

    // See the GIF specification for details on the composition
    // of this packed byte. We set the local color table flag,
    // and unset the interlace and sort flags in the upper 4 bits.
    // We encode the size of the local color table in the lower 3 
    // bits. This is just the bit depth of the color table minus 1.
    inline uint8_t
    get_image_descriptor_packed_byte(std::size_t local_color_table_bit_depth) {
        assert (local_color_table_bit_depth <= 8);
        uint8_t encoded_table_size = static_cast<uint8_t>(local_color_table_bit_depth - 1);
        assert (encoded_table_size <= 7);
        return 0x80 | encoded_table_size;    
    }
}

#endif
