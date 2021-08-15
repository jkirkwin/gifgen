// A helper program to read in a GIF file and print out
// the observed parameters. For use debugging outputs of 
// the gifgen application.

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <format>

uint16_t construct_numeric_field(uint8_t lsb, uint8_t msb) {
    return (uint16_t(msb) << 8) + lsb;
}

// Print out instructions on how to run the program.
void print_usage() {
    std::cout << "USAGE:\tdegif <input.gif>" << std::endl; 
}

std::ostream& error() {
    return std::cout << "ERROR: ";
}

class gif_decoder {
public:
    
    // Reads in the provided GIF file and prints out the 
    // information that was found.
    void read_input_file(std::string filename) {
        if (std::filesystem::exists(filename)) {
            std::cout << "Reading file: " << filename << std::endl;
        }
        else {
            std::cout << "File " << filename << " not found." << std::endl;
        }

        // Read the entire file into memory for convenience. The files that we're
        // debugging with this program are small, so there is not a prohibitive
        // performance cost here.
        std::ifstream in_file(filename, std::ios::binary);
        binary_content = std::vector<uint8_t>(std::istreambuf_iterator<char>(in_file), {});

        // Verify the static header blocks
        if (verify_gif_header() 
        && verify_screen_descriptor() 
        && verify_netscape_ext()) {

            // Loop until we hit the trailer block
            while (binary_content[current_byte_offset] != TRAILER_BYTE) {
                bool verify_image_success = 
                    verify_graphics_ext() &&
                    verify_image_descriptor() &&
                    verify_local_color_table() &&
                    verify_image_data();
                if (!verify_image_success) {
                    return;
                }
            }

            // Check the trailer
            if (verify_trailer()) {
                std::cout << "All validation steps succeeded." << std::endl;
            }
        }
    }

private:
    constexpr static std::size_t HEADER_SIZE = 6;
    constexpr static std::size_t SCREEN_DESCRIPTOR_SIZE = 7;
    constexpr static std::size_t NETSCAPE_EXT_SIZE = 19;
    constexpr static std::size_t GRAPHIC_EXT_SIZE = 8;
    constexpr static std::size_t IMAGE_DESCRIPTOR_SIZE = 10;

    constexpr static uint8_t TRAILER_BYTE = 0x3B;

    std::vector<uint8_t> binary_content;
    std::size_t current_byte_offset;
    std::size_t local_color_table_size;
    std::size_t total_image_data_size; 

    // Validates the GIF file signature and the version.
    // Returns true if the file holds the expected value.
    bool verify_gif_header() {
        std::string expected_header("GIF89a");

        bool match = std::equal(expected_header.begin(), expected_header.end(), binary_content.begin());
        if (!match) {
            error() << "Header does not match. " << std::endl;
            return false;
        }

        current_byte_offset = HEADER_SIZE;
        return true;
    } 

    bool verify_screen_descriptor() {
        std::cout << "Logical Screen Descriptor: " << std::endl;

        uint8_t width_lsb = binary_content[current_byte_offset];
        uint8_t width_msb = binary_content[current_byte_offset + 1];

        uint16_t width = construct_numeric_field(width_lsb, width_msb);
        std::cout << "\tScreen width: " << width << std::endl;

        uint8_t height_lsb = binary_content[current_byte_offset + 2];
        uint8_t height_msb = binary_content[current_byte_offset + 3];

        uint16_t height = construct_numeric_field(height_lsb, height_msb);
        std::cout << "\tScreen height: " << height << std::endl;

        uint8_t packed_fields = binary_content[current_byte_offset + 4];
        uint8_t global_color_flag = packed_fields & 0x80;
        uint8_t color_resolution = (packed_fields & 0x70) >> 4;
        uint8_t sort_flag = packed_fields & 0x08;
        // uint8_t global_color_table_size = packed_fields & 0x07; // We don't use this.

        if (global_color_flag != 0) {
            error() << "Global color table flag is set" << std::endl;
            return false;
        }
        if (color_resolution != 7) {
            error() << "Incorrect color resolution. Expected 0b111, but got (int value) " << color_resolution << std::endl;
            return false;
        }
        if (sort_flag != 0) {
            error() << "Sort flag is set" << std::endl;
            return false;
        }

        uint8_t background_index = binary_content[current_byte_offset + 5];
        if (background_index != 0) {
            error() << "Background index is set" << std::endl;
            return false;
        }

        uint8_t pixel_aspect_ratio = binary_content[current_byte_offset + 6];
        if (pixel_aspect_ratio != 0) {
            error() << "Pixel aspect ratio is set" << std::endl;
            return false;
        }

        current_byte_offset += SCREEN_DESCRIPTOR_SIZE;

        return true;
    }

    bool verify_netscape_ext() {
        std::cout << "Netscape extension:" << std::endl;

        if (binary_content[current_byte_offset] != 0x21) {
            error() << "Extension introducer is missing." << std::endl;
            return false;
        }
        if (binary_content[current_byte_offset + 1] != 0xFF) {
            error() << "Incorrect control label" << std::endl;
            return false;
        }
        if (binary_content[current_byte_offset + 2] != 0x0B) {
            error() << "Incorrect block size (1)" << std::endl;
            return false;
        }

        std::string netscape_id("NETSCAPE2.0");
        auto it = &binary_content[current_byte_offset + 3];
        bool match = std::equal(netscape_id.begin(), netscape_id.end(), it);
        if (!match) {
            error() << "Header/version string mismatch" << std::endl;
        }

        if (binary_content[current_byte_offset + 14] != 3) {
            error() << "Incorrect block size (2)" << std::endl;
            return false;
        }
        if (binary_content[current_byte_offset + 15] != 1) {
            error() << "Incorrect block ID (2)" << std::endl;
            return false;
        }

        uint8_t loop_lsb = binary_content[current_byte_offset + 16];
        uint8_t loop_msb = binary_content[current_byte_offset + 17];
        uint16_t loop_count = construct_numeric_field(loop_lsb, loop_msb);
        std::cout << "\tLoop count: " <<  loop_count << " (0 means infinite looping)" << std::endl;

        if (binary_content[current_byte_offset + 18] != 0) {
            error() << "Missing block terminator" << std::endl;
            return false;
        }

        current_byte_offset += NETSCAPE_EXT_SIZE;
        return true;
    }

    bool verify_graphics_ext() {
        std::cout << "Graphic control extension:" << std::endl;

        if (binary_content[current_byte_offset] != 0x21) {
            error() << "Extension introducer is missing." << std::endl;
            return false;
        }
        if (binary_content[current_byte_offset + 1] != 0xF9) {
            error() << "Incorrect control label" << std::endl;
            return false;
        }

        // Most of this is meaningless for us, so we just pull out
        // the timing delay.
        uint8_t delay_lsb = binary_content[current_byte_offset + 4];
        uint8_t delay_msb = binary_content[current_byte_offset + 5];
        uint16_t delay = construct_numeric_field(delay_lsb, delay_msb);
        std::cout << "\tDelay: " << delay << " (100ths of a second)" << std::endl;

        if (binary_content[current_byte_offset + 7] != 0) {
            error() << "Block terminator not found in expected position" << std::endl;
            return false;
        }

        current_byte_offset += GRAPHIC_EXT_SIZE;
        return true;
    }

    bool verify_image_descriptor() {
        std::cout << "Image Descriptor: " << std::endl;

        uint8_t image_separator = binary_content[current_byte_offset];
        if (image_separator != 0x2C) {
            error() << "No image separator found to indicate start of image descriptor" << std::endl;
            return false;
        }

        uint8_t image_left_lsb = binary_content[current_byte_offset + 1];
        uint8_t image_left_msb = binary_content[current_byte_offset + 2];
        uint16_t left = construct_numeric_field(image_left_lsb, image_left_msb);
        if (left != 0) {
            std::cout << "\tImage left position: " << left << std::endl;
        }

        uint8_t image_top_lsb = binary_content[current_byte_offset + 3];
        uint8_t image_top_msb = binary_content[current_byte_offset + 4];
        uint16_t top = construct_numeric_field(image_top_lsb, image_top_msb);
        if (top != 0) {
            std::cout << "\tImage top position: " << top << std::endl;
        }

        uint8_t width_lsb = binary_content[current_byte_offset + 5];
        uint8_t width_msb = binary_content[current_byte_offset + 6];
        uint16_t width = construct_numeric_field(width_lsb, width_msb);
        std::cout << "\tImage width: " << width << std::endl;

        uint8_t height_lsb = binary_content[current_byte_offset + 7];
        uint8_t height_msb = binary_content[current_byte_offset + 8];
        uint16_t height = construct_numeric_field(height_lsb, height_msb);
        std::cout << "\tImage height: " << height << std::endl;
        
        uint8_t bit_fields = binary_content[current_byte_offset + 9];
        uint8_t local_color_table_flag = bit_fields & 0x80;
        uint8_t interlace_flag = bit_fields & 0x40;
        uint8_t sort_flag = bit_fields & 0x20;
        uint8_t encoded_color_table_size = bit_fields & 0x07;

        if (local_color_table_flag == 0) {
            error() << "Local color table flag is unset" << std::endl;
            return false;
        }
        if (interlace_flag != 0) {
            error() << "Interlace flag is set" << std::endl;
            return false;
        }
        if (sort_flag != 0) {
            error() << "Sort flag is set" << std::endl;
            return false;
        }

        local_color_table_size = 1 << (encoded_color_table_size + 1);
        std::cout << "\tLocal color table size: " << local_color_table_size << std::endl;

        current_byte_offset += IMAGE_DESCRIPTOR_SIZE;

        return true;
    }

    bool verify_local_color_table() {
        std::cout << "Local Color Table: " << std::endl;

        auto local_color_table_bytes = 3 * local_color_table_size;
        auto r_last = binary_content[current_byte_offset + local_color_table_bytes - 3];
        auto g_last = binary_content[current_byte_offset + local_color_table_bytes - 2];
        auto b_last = binary_content[current_byte_offset + local_color_table_bytes - 1];
        std::cout << "\tLast color in color table is " 
                  << fmt::format("{:#04X} {:#04X} {:#04X}", r_last, g_last, b_last) 
                  << std::endl;

        current_byte_offset += local_color_table_bytes;     

        return true;
    }

    bool verify_image_data() {
        std::cout << "Image sub-blocks: " << std::endl;

        auto lzw_code_size = binary_content[current_byte_offset];
        if (lzw_code_size != 8) {
            error() << "LZW Code Size is set to " << (int)lzw_code_size << std::endl;
            return false;
        }

        auto sub_block_header_index = current_byte_offset + 1;
        uint8_t sub_block_size = binary_content[sub_block_header_index];
        std::size_t total_size = 0;
        int n = 0;
        while(sub_block_size != 0) {
            // Record data about this sub-block
            total_size += sub_block_size;
            ++n;

            // Move the header index to the header of the next block
            sub_block_header_index += sub_block_size + 1;
            sub_block_size = binary_content[sub_block_header_index];
        }
        std::cout << "\tFound terminator sub-block after " 
                  << n << " data blocks holding " << total_size 
                  << " lzw-encoded bytes" << std::endl;
        
        // There were total_size content bytes, n+1 block header bytes, 
        // and the lzw code size
        current_byte_offset += total_size + n + 2;

        return true;
    }

    bool verify_trailer() {
        std::cout << "Trailer: " << std::endl;

        if (binary_content.size() != current_byte_offset + 1) {
            error() << "Data size is inconsistent with computed offset. There are " 
                    << binary_content.size() 
                    << " bytes of data, but the trailer should be at index " 
                    << current_byte_offset 
                    << std::endl; 
            return false;
        }

        auto last_byte = binary_content.at(current_byte_offset); 
        if (last_byte != TRAILER_BYTE) {
            error() << fmt::format("Incorrect trailer value: {:#04x}. Expected 0X3B.", last_byte) << std::endl;
        }
        else {
            std::cout << "\tFound trailer byte" << std::endl;
        }

        return true;
    }

};

int main(int argc, char** argv) {
    if (argc != 2) {
        print_usage();
        return 1;
    }

    std::string input_file_name(argv[1]);
    gif_decoder decoder;
    decoder.read_input_file(input_file_name);

    return 0;
}