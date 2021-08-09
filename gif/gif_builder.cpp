#include "gif_builder.hpp"
#include "gif_data_format.hpp"
#include "../lzw/include/lzw.hpp" // TODO Fix this
#include <cassert>

namespace gif {

    // Splits a 16-bit number into its component bytes as chars.
    // chars are used instead of uchars because it simplifies client
    // code which needs to write the results to an ostream in bulk.
    std::pair<char, char> split_numeric_field(uint16_t n) {
        char lsb(n & 0xFF);
        char msb((n >> 8) & 0xFF);
        return std::make_pair(lsb, msb);
    }

    void gif_builder::write_gif_header() {
        out_file << "GIF89a";
    }

    void gif_builder::write_screen_descriptor() {
        // Break dimension values into bytes
        auto [width_lsb, width_msb] = split_numeric_field(width);
        auto [height_lsb, height_msb] = split_numeric_field(height);

        // Construct the block and write it to the file
        std::vector<char> screen_descriptor_block {
            width_lsb, width_msb,
            height_lsb, height_msb,
            SCREEN_DESCRIPTOR_PACKED_BYTE,
            0x00, // Background color. Not used. 
            0x00  // Pixel aspect ratio. Not used.
        };

        assert (screen_descriptor_block.size() == SCREEN_DESCRIPTOR_SIZE);
        out_file.write(screen_descriptor_block.data(), SCREEN_DESCRIPTOR_SIZE);
    }

    void gif_builder::write_image_descriptor(const image::rgb_image_view_t& image_view,
                                             const palettize::color_table& local_color_table) {
        // Break dimension values into bytes
        auto image_width = image_view.width();
        auto image_height = image_view.height();
        
        assert (static_cast<std::size_t>(image_width) == width);
        assert (static_cast<std::size_t>(image_height) == height);

        auto [width_lsb, width_msb] = split_numeric_field(image_width);
        auto [height_lsb, height_msb] = split_numeric_field(image_height);

        // Pack the color bit fields into a byte. See the GIF spec
        // for more information.
        auto bit_depth = local_color_table.min_bit_depth();
        char color_bit_fields = get_image_descriptor_packed_byte(bit_depth);

        // Construct the block and write it to the file
        std::vector<char> image_descriptor_block {
            IMAGE_SEPARATOR_BYTE,
            0x00, 0x00, // Left offset
            0x00, 0x00, // Top offset
            width_lsb, width_msb,
            height_lsb, height_msb,
            color_bit_fields
        };

        assert (image_descriptor_block.size() == IMAGE_DESCRIPTOR_SIZE);
        out_file.write(image_descriptor_block.data(), IMAGE_DESCRIPTOR_SIZE);
    }

    void gif_builder::write_local_color_table(const palettize::color_table& local_color_table) {
        // We might have any number of colors in the palette up to 256, but the
        // size of the block is encoded as a power of 2, so we must round up to
        // the next power of 2.
        auto encoded_block_size = 1u << local_color_table.min_bit_depth();
        auto block_size = encoded_block_size * 3; // 3 bytes per color
        std::vector<char> color_table_block(block_size);

        for (std::size_t i = 0; i < local_color_table.size(); ++i) {
            auto pixel = local_color_table.at(i);
            char r = static_cast<char>(pixel[0]);
            char g = static_cast<char>(pixel[1]);
            char b = static_cast<char>(pixel[2]);

            auto block_index = 3 * i;
            color_table_block.at(block_index)     = r;
            color_table_block.at(block_index + 1) = g;
            color_table_block.at(block_index + 2) = b;
        }

        assert (color_table_block.size() == block_size);
        out_file.write(color_table_block.data(), block_size);
    }

    // Encodes the image as LZW-compressed color table indices, packages up the
    // resulting codes into sub-blocks, and writes those blocks to the output file.
    void gif_builder::write_image_data(const image::rgb_image_view_t& image_view,
                                       const palettize::color_table& local_color_table) {
        // The first byte of the image block tells the decoder how many bits
        // to use for its LZW dictionary.
        out_file << LZW_CODE_SIZE;

        // The remainder of the image block is made up of data sub-blocks full
        // of LZW-compressed image data. The image data must first be encoded
        // as color table indices. Then, we can set the LZW encoder to forward
        // directly to a buffer that packs the sub-blocks appropriately.
        auto index_list = palettize::palettize_image(image_view, local_color_table);

        // The block buffer should never have anything left overfrom  previous frames.
        assert (block_buffer.current_block_size() == 0);
        lzw::lzw_encoder encoder(LZW_CODE_SIZE, block_buffer); 
        encoder.encode(index_list.begin(), index_list.end());
        encoder.flush();

        // Write out any remaining data from the buffer in a smaller
        // sub-block.
        if (block_buffer.current_block_size() > 0) {
            block_buffer.write_current_block();
        }

        // Terminate the image block with an empty sub-block.
        assert (block_buffer.current_block_size() == 0);
        block_buffer.write_current_block();
    }

    void gif_builder::write_gif_trailer() {
        out_file << GIF_TRAILER_BYTE;
    }

    gif_builder::gif_builder(std::ostream& out, std::size_t width, std::size_t height) :
            out_file(out), 
            block_buffer(out_file),
            width(width), 
            height(height), 
            stream_complete(false) {
        assert (width > 0);
        assert (height > 0);
        assert (width <= std::numeric_limits<uint16_t>::max());
        assert (height <= std::numeric_limits<uint16_t>::max());
        write_gif_header();
        write_screen_descriptor();
    }

    gif_builder::~gif_builder() {
        if (!stream_complete)  {
            complete_stream();
        }
    }


    gif_builder& gif_builder::add_frame(const image::rgb_image_view_t& image_view) {
        // For each frame, we need to encode:
        // 1. Image Descriptor
        // 2. Local Color Table
        // 3. Index-encoded, LZW-compressed image data
                
        auto color_palette = palettize::create_color_table(image_view);
        write_image_descriptor(image_view, color_palette);
        write_local_color_table(color_palette);
        write_image_data(image_view, color_palette);
        
        return *this;
    }

    void gif_builder::complete_stream() {
        assert (!stream_complete);
        stream_complete = true;
        write_gif_trailer();
    }

}