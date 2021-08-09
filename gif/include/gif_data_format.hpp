#ifndef GIF_DATA_FORMAT_HPP
#define GIF_DATA_FORMAT_HPP

#include <cstdint>

namespace gif {
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
}

#endif
