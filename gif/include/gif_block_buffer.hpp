#ifndef GIF_BLOCK_BUFFER_HPP
#define GIF_BLOCK_BUFFER_HPP

#include <array>
#include <ostream>
#include <cstdlib>
#include "gif_data_format.hpp"

namespace gif {
    
    // Creates data sub-blocks (see the GIF spec for details)
    // by buffering incoming bytes until the maximum sub-block
    // size is reached, or the data is requested to be packaged
    // up explicitly.
    // Once a sub-block is packaged, it is written to the output 
    // file in one operation.    
    class gif_block_buffer {
    public:

        // Construct a new gif_block_buffer whose output will be 
        // written to the provided output stream. 
        gif_block_buffer(std::ostream& out);

        // Buffers are neither copyable nor moveable.
        gif_block_buffer(const gif_block_buffer&) = delete;
        gif_block_buffer& operator=(const gif_block_buffer&) = delete;
        gif_block_buffer(gif_block_buffer&&) = delete;
        gif_block_buffer& operator=(gif_block_buffer&&) = delete;

        // Destory the gif_block_buffer. If there is any data 
        // still in the buffer, it will be written in a smaller
        // than maximal block before the buffer is destroyed. 
        ~gif_block_buffer();

        // Inserts a byte into the current sub-block. If this causes
        // the sub-block's size to reach the maximum allowed, it will
        // be written to the output file and a new sub-block will be
        // started.
        gif_block_buffer& operator<<(uint8_t byte);

        // Returns the number of bytes in the payload of the current
        // sub-block, not including the one-byte header.
        std::size_t current_block_size() const;

        // Forces the current (non-full) sub-block to be written 
        // to the output file as-is.
        void write_current_block();

    private:
        // Each sub-block includes its payload and a one-byte header.
        static constexpr std::size_t SUB_BLOCK_BUFFER_SIZE 
                                        = MAX_IMAGE_SUB_BLOCK_SIZE + 1;

        std::ostream& out_file;
        std::array<char, SUB_BLOCK_BUFFER_SIZE> buffer;
    };

}

#endif
