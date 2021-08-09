#include <cassert>
#include "sub_block_buffer.hpp"

namespace gif {

    sub_block_buffer::sub_block_buffer(std::ostream& out) : 
            out_file(out), 
            buffer() {
        // Ensure the size is zero to start with
        buffer.at(0) = 0;
    }

    sub_block_buffer::~sub_block_buffer() {
        if (current_block_size() > 0) {
            write_current_block();
        }
    }

    sub_block_buffer& sub_block_buffer::operator<<(uint8_t byte) {
        assert (current_block_size() < MAX_IMAGE_SUB_BLOCK_SIZE);

        buffer.at(current_block_size() + 1) = char(byte);
        ++buffer.at(0);

        if (current_block_size() == MAX_IMAGE_SUB_BLOCK_SIZE) {
            write_current_block();
        }

        return *this;
    }

    std::size_t sub_block_buffer::current_block_size() const {
        return static_cast<uint8_t>(buffer.at(0));
    }

    void sub_block_buffer::write_current_block() {
        assert (current_block_size() <= MAX_IMAGE_SUB_BLOCK_SIZE);
        
        // Write the header, followed by size bytes of data. 
        // The size is stored in the buffer already so we don't
        // need to update it before writing.
        auto bytes = current_block_size() + 1;
        out_file.write(buffer.begin(), bytes);

        // Reset the size to 0
        buffer.at(0) = 0;
    }
}
