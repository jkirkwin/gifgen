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

constexpr uint8_t LZW_CODE_SIZE = 8;

// TODO move these to a GIF header file?
constexpr std::size_t MAX_IMAGE_SUB_BLOCK_SIZE = 255;
constexpr std::size_t SCREEN_DESCRIPTOR_SIZE = 7;
constexpr std::size_t IMAGE_DESCRIPTOR_SIZE = 10;

constexpr uint8_t IMAGE_SEPARATOR_BYTE = 0x2C;
constexpr uint8_t GIF_TRAILER_BYTE = 0x3B;

    // See the GIF specification for details on the composition
    // of this packed byte. We set the global color table flag
    // to 0, the color resolution to 8 (encoded as 7), the sort
    // flag is set to 0, and the global color table size is 
    // left as all zeros as well.
constexpr std::size_t SCREEN_DESCRIPTOR_PACKED_BYTE = 0x70;

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
        SCREEN_DESCRIPTOR_PACKED_BYTE,
        0x00, // Background color. Not used. 
        0x00  // Pixel aspect ratio. Not used.
    };

    assert (screen_descriptor_block.size() == SCREEN_DESCRIPTOR_SIZE);
    out_file.write(screen_descriptor_block.data(), SCREEN_DESCRIPTOR_SIZE);
}

void write_image_descriptor(std::ofstream& out_file, 
                            const image::rgb_image_view_t& image_view,
                            const palettize::color_table& local_color_table) {
    // Break dimension values into bytes
    auto width = image_view.width();
    auto height = image_view.height();
    auto [width_lsb, width_msb] = split_numeric_field(width);
    auto [height_lsb, height_msb] = split_numeric_field(height);

    // See the GIF specification for details on the composition
    // of this packed byte. We set the local color table flag,
    // and unset the interlace and sort flags in the upper 4 bits.
    // We encode the size of the local color table in the lower 3 
    // bits. This is just the bit depth of the color table minus 1.
    uint8_t encoded_table_size = local_color_table.min_bit_depth() - 1; // TODO Move this computation into the same place as the constants at the top of the file.
    assert (encoded_table_size <= 7);
    char local_color_fields = 0x80 | encoded_table_size;

    // Construct the block and write it to the file
    std::vector<char> image_descriptor_block {
        IMAGE_SEPARATOR_BYTE,
        0x00, 0x00, // Left offset
        0x00, 0x00, // Top offset
        width_lsb, width_msb,
        height_lsb, height_msb,
        local_color_fields
    };

    assert (image_descriptor_block.size() == IMAGE_DESCRIPTOR_SIZE);
    out_file.write(image_descriptor_block.data(), IMAGE_DESCRIPTOR_SIZE);
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

// TODO Remove this and replace it with something less gross.
// A simple wrapper around a vector that pushes data
// received via operator<<.
template<class T>
struct vector_insertion_wrapper { // TODO Make a proper class for this
    std::vector<T>* vec;

    // Inserts a value of type T2 into the vector. T must be construct-able
    // from a value of type T2.
    template <class T2>
    vector_insertion_wrapper& operator<<(T2 datum) {
        vec->push_back(T(datum));
        return *this;
    }
};

// Encodes the provided list of color table indices into an LZW data stream,
// packages up the resulting codes into blocks, and writes those blocks to
// the output file.
void write_image_data(std::ofstream& out_file, const std::vector<uint8_t>& indices) {
    // TODO We need a buffer for the bytes? For now, just insert all of them into 
    // a vector and iterate over the vector.
    std::vector<char> encoded_data;
    vector_insertion_wrapper<char> vec_wrapper {&encoded_data};

    lzw::lzw_encoder encoder(LZW_CODE_SIZE, vec_wrapper); // TODO Clean this stuff up, and consider using a streaming approach where the thing that's downstream from the lzw encoder automatically writes a subblock as soon as its buffer is full.
    encoder.encode(indices.begin(), indices.end());
    encoder.flush();

    std::cout << "Encoded " << indices.size() << " indices into " << encoded_data.size() << " bytes" << std::endl;

    out_file << LZW_CODE_SIZE;

    // GIF supports sub-blocks of up to 255 bytes. Write the encoded data 
    // out in blocks of maximal size until all data has been written.
    auto data_size = encoded_data.size();
    auto remaining_bytes = data_size; 
    while (remaining_bytes > 0) {
        // For each block, write the number of data bytes that follow
        std::size_t sub_block_size = std::min(remaining_bytes, MAX_IMAGE_SUB_BLOCK_SIZE);
        assert (sub_block_size <= 255);
        uint8_t sub_block_size_byte = static_cast<uint8_t>(sub_block_size);


        // Write the data for the sub-block
        out_file << sub_block_size_byte; // Sub-block header
        auto starting_index = data_size - remaining_bytes;
        auto data_ptr = encoded_data.data() + starting_index;
        out_file.write(data_ptr, sub_block_size);

        remaining_bytes -= sub_block_size;
    }

    // No more data follows. Signal the end of the image data with
    // an empty data sub-block.
    out_file << uint8_t(0); 
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
    out_file << GIF_TRAILER_BYTE;
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