// This is the entry point for the gifgen application.
//
// See CMake.txt for details about the build.
// See README.txt for details about the project and how 
// to run this code.

#include <cassert>
#include <iostream>
#include <fstream>
#include "args.hpp"
#include "image_io.hpp"
#include "image_utils.hpp"
#include "palettize.hpp"
#include "lzw.hpp"
#include <format> // TODO remove
#include "sub_block_buffer.hpp"
#include "gif_data_format.hpp"

// Read in a JPEG or PNG image and return its GIL representation.
image::rgb_image_t read_image (const std::string& filename, args::input_file_type file_type) {
    image::rgb_image_t img;
    if (file_type == args::input_file_type::PNG) {
        image::read_png_image(filename, img);
    }
    else {
        assert (file_type == args::input_file_type::JPEG);
        image::read_jpeg_image(filename, img);
    }

    std::cout << "Read in image with dimensions " << img.width() << "x" << img.height() << std::endl;

    return img;
}

// Splits a 16-bit number into its component bytes as chars.
// chars are used instead of uchars because it simplifies client
// code which needs to write the results to a file.
std::pair<char, char> split_numeric_field(uint16_t n) {
    char lsb(n & 0xFF);
    char msb((n >> 8) & 0xFF);
    return std::make_pair(lsb, msb);
}

void write_gif_header(std::ofstream& out_file) {
    out_file << "GIF89a";
}

void write_screen_descriptor(std::ofstream& out_file, const image::rgb_image_view_t& image_view) {
    // Break dimension values into bytes
    auto width = image_view.width();
    auto height = image_view.height();
    auto [width_lsb, width_msb] = split_numeric_field(width);
    auto [height_lsb, height_msb] = split_numeric_field(height);

    // Construct the block and write it to the file
    std::vector<char> screen_descriptor_block {
        width_lsb, width_msb,
        height_lsb, height_msb,
        gif::SCREEN_DESCRIPTOR_PACKED_BYTE,
        0x00, // Background color. Not used. 
        0x00  // Pixel aspect ratio. Not used.
    };

    assert (screen_descriptor_block.size() == gif::SCREEN_DESCRIPTOR_SIZE);
    out_file.write(screen_descriptor_block.data(), gif::SCREEN_DESCRIPTOR_SIZE);
}

void write_image_descriptor(std::ofstream& out_file, 
                            const image::rgb_image_view_t& image_view,
                            const palettize::color_table& local_color_table) {
    // Break dimension values into bytes
    auto width = image_view.width();
    auto height = image_view.height();
    auto [width_lsb, width_msb] = split_numeric_field(width);
    auto [height_lsb, height_msb] = split_numeric_field(height);

    // Pack the color bit fields into a byte. See the GIF spec
    // for more information.
    char packed_byte = 
        gif::get_image_descriptor_packed_byte(local_color_table.min_bit_depth());

    // Construct the block and write it to the file
    std::vector<char> image_descriptor_block {
        gif::IMAGE_SEPARATOR_BYTE,
        0x00, 0x00, // Left offset
        0x00, 0x00, // Top offset
        width_lsb, width_msb,
        height_lsb, height_msb,
        packed_byte // Color info
    };

    assert (image_descriptor_block.size() == gif::IMAGE_DESCRIPTOR_SIZE);
    out_file.write(image_descriptor_block.data(), gif::IMAGE_DESCRIPTOR_SIZE);
}

// Writes the local color table to the data stream
void write_local_color_table(std::ofstream& out_file, const palettize::color_table& color_palette) {
    // We might have any number of colors in the palette up to 256, but
    // the size of the block is encoded as a power of 2, so we use the
    // smallest power of 2 that is at least big enough to house the color 
    // table.
    auto encoded_block_size = 1u << color_palette.min_bit_depth();
    auto block_size = encoded_block_size * 3; // 3 bytes per color
    std::vector<char> color_table_block(block_size);

    std::cout << "Writing color table of size " << color_palette.size() 
              << " with block big enough for " << encoded_block_size << " colors"
              << std::endl;

    for (std::size_t i = 0; i < color_palette.size(); ++i) {
        auto pixel = color_palette.at(i);
        char r = static_cast<char>(pixel[0]);
        char g = static_cast<char>(pixel[1]);
        char b = static_cast<char>(pixel[2]);

        auto block_index = 3 * i;
        color_table_block.at(block_index)     = r;
        color_table_block.at(block_index + 1) = g;
        color_table_block.at(block_index + 2) = b;
    }

    // TODO Print last pixel to verify decoding. Remove this eventually.
    auto last_color = color_palette.at(color_palette.size() - 1); 
    auto r_last = last_color[0];
    auto g_last = last_color[1];
    auto b_last = last_color[2];
    std::cout << "Last color in color table is " 
              << fmt::format("{:#04X} {:#04X} {:#04X}", r_last, g_last, b_last) 
              << std::endl;

    assert (color_table_block.size() == block_size);
    out_file.write(color_table_block.data(), block_size);
}

// Encodes the provided list of color table indices into an LZW data stream,
// packages up the resulting codes into sub-blocks, and writes those blocks to
// the output file.
void write_image_data(std::ofstream& out_file, const std::vector<uint8_t>& indices) {
    // The first byte of the image block tells the decoder how many bits
    // to use for its LZW dictionary.
    out_file << gif::LZW_CODE_SIZE;

    // The remainder of the image block is made up of data sub-blocks
    // full of LZW-compressed image data. We set the LZW encoder to forward
    // directly to a buffer that packs the sub-blocks appropriately.
    gif::sub_block_buffer block_buffer(out_file);
    lzw::lzw_encoder encoder(gif::LZW_CODE_SIZE, block_buffer); 
    encoder.encode(indices.begin(), indices.end());
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

// Encodes the given image as the next frame in the data stream.
void encode_frame(std::ofstream& out_file, const image::rgb_image_view_t& image_view) {
    // Create a color table for the image and write the image
    // descriptor and color table to the stream.
    auto color_palette = palettize::create_color_table(image_view);
    write_image_descriptor(out_file, image_view, color_palette);
    write_local_color_table(out_file, color_palette);

    // Quantize the image data to use the computed color palette, and
    // encode the resulting indices via LZW. 
    auto index_list = palettize::palettize_image(image_view, color_palette);
    write_image_data(out_file, index_list);
}

void write_gif_trailer(std::ofstream& out_file) {
    out_file << gif::GIF_TRAILER_BYTE;
}

int main(int argc, char **argv) {
    auto args = args::parse_arguments(argc, argv);

    if (!args.other_args.empty()) {
        std::cout << "Extra string arguments: ";
        for (const auto& s : args.other_args) {
            std::cout << s << " ";
        }
        std::cout << std::endl;
    }

    // TODO Add logic to check the validity of the given files
    //  - Check that the input file exists
    //  - Check that the input file is a jpeg/png image as appropriate

    // Read in the image
    auto img = read_image(args.file_name, args.file_type);
    auto image_view = boost::gil::view(img);

    // Open the output file and write the single-image data
    std::ofstream result_file(args.output_file_name, std::ios::out | std::ios::binary);

    write_gif_header(result_file);
    write_screen_descriptor(result_file, image_view);
    encode_frame(result_file, image_view);
    write_gif_trailer(result_file);

    return 0;
}