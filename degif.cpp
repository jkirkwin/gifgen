// A helper program to read in a GIF file and print out
// the observed parameters. For use debugging the main
// gifgen application.

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <format>

// TODO Clean this file up or remove it before submitting

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

        // Check over each portion of the file until we find an error.
        if (verify_gif_header() 
        && verify_screen_descriptor()
        && verify_image_descriptor()
        && verify_local_color_table()
        && verify_image_data()
        && verify_trailer()) {
            std::cout << "All validation steps succeeded." << std::endl;
        }
    }

private:
    constexpr static std::size_t HEADER_SIZE = 6;
    constexpr static std::size_t SCREEN_DESCRIPTOR_SIZE = 7;
    constexpr static std::size_t IMAGE_DESCRIPTOR_OFFSET = HEADER_SIZE + SCREEN_DESCRIPTOR_SIZE;
    constexpr static std::size_t IMAGE_DESCRIPTOR_SIZE = 10;
    constexpr static std::size_t COLOR_TABLE_OFFSET = IMAGE_DESCRIPTOR_OFFSET + IMAGE_DESCRIPTOR_SIZE;


    std::vector<uint8_t> binary_content;
    std::size_t local_color_table_size;  
    std::size_t local_color_table_bytes;  
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

        return true;
    } 

    bool verify_screen_descriptor() {
        std::cout << "Logical Screen Descriptor: " << std::endl;

        auto start = HEADER_SIZE;

        uint8_t width_lsb = binary_content[start];
        uint8_t width_msb = binary_content[start + 1];

        uint16_t width = construct_numeric_field(width_lsb, width_msb);
        std::cout << "\tScreen width: " << width << std::endl;

        uint8_t height_lsb = binary_content[start + 2];
        uint8_t height_msb = binary_content[start + 3];

        uint16_t height = construct_numeric_field(height_lsb, height_msb);
        std::cout << "\tScreen height: " << height << std::endl;

        uint8_t packed_fields = binary_content[start + 4];
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

        uint8_t background_index = binary_content[start + 5];
        if (background_index != 0) {
            error() << "Background index is set" << std::endl;
            return false;
        }

        uint8_t pixel_aspect_ratio = binary_content[start + 6];
        if (pixel_aspect_ratio != 0) {
            error() << "Pixel aspect ratio is set" << std::endl;
            return false;
        }

        return true;
    }

    bool verify_image_descriptor() {
        std::cout << "Image Descriptor: " << std::endl;

        auto start = IMAGE_DESCRIPTOR_OFFSET;

        uint8_t image_separator = binary_content[start];
        if (image_separator != 0x2C) {
            error() << "No image separator found to indicate start of image descriptor" << std::endl;
            return false;
        }

        uint8_t image_left_lsb = binary_content[start + 1];
        uint8_t image_left_msb = binary_content[start + 2];
        uint16_t left = construct_numeric_field(image_left_lsb, image_left_msb);
        if (left != 0) {
            std::cout << "\tImage left position: " << left << std::endl;
        }

        uint8_t image_top_lsb = binary_content[start + 3];
        uint8_t image_top_msb = binary_content[start + 4];
        uint16_t top = construct_numeric_field(image_top_lsb, image_top_msb);
        if (top != 0) {
            std::cout << "\tImage top position: " << top << std::endl;
        }

        uint8_t width_lsb = binary_content[start + 5];
        uint8_t width_msb = binary_content[start + 6];
        uint16_t width = construct_numeric_field(width_lsb, width_msb);
        std::cout << "\tImage width: " << width << std::endl;

        uint8_t height_lsb = binary_content[start + 7];
        uint8_t height_msb = binary_content[start + 8];
        uint16_t height = construct_numeric_field(height_lsb, height_msb);
        std::cout << "\tImage height: " << height << std::endl;
        
        uint8_t bit_fields = binary_content[start + 9];
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

        return true;
    }

    bool verify_local_color_table() {
        std::cout << "Local Color Table: " << std::endl;

        auto start = COLOR_TABLE_OFFSET;

        auto r0 = binary_content[start];
        auto g0 = binary_content[start + 1];
        auto b0 = binary_content[start + 2];
        if (r0 != 0x00 || g0 != 0x00 || b0 != 0x00) {
            error() << "Second color table entry is not white" << std::endl;
            return false;
        }
        
        auto r1 = binary_content[start + 3];
        auto g1 = binary_content[start + 4];
        auto b1 = binary_content[start + 5];
        if (r1 != 0xFF || g1 != 0xFF || b1 != 0xFF) {
            error() << "Second color table entry is not white" << std::endl;
            return false;
        }

        local_color_table_bytes = 3 * local_color_table_size;
        auto r_last = binary_content[start + local_color_table_bytes - 3];
        auto g_last = binary_content[start + local_color_table_bytes - 2];
        auto b_last = binary_content[start + local_color_table_bytes - 1];
        std::cout << "\tLast color in color table is " 
                  << fmt::format("{:#04X} {:#04X} {:#04X}", r_last, g_last, b_last) 
                  << std::endl;

        return true;
    }

    bool verify_image_data() {
        std::cout << "Image sub-blocks: " << std::endl;

        auto start = COLOR_TABLE_OFFSET + local_color_table_bytes;
        auto lzw_code_size = binary_content[start];
        if (lzw_code_size != 8) {
            error() << "LZW Code Size is set to " << (int)lzw_code_size << std::endl;
            return false;
        }

        auto sub_block_header_index = start + 1;
        uint8_t sub_block_size = binary_content[sub_block_header_index];
        std::size_t total_size = 0;
        int n = 0;
        while(sub_block_size != 0) {
            // Record data about this sub-block
            std::cout << "\tData block found of size " << (int)sub_block_size << std::endl;
            total_size += sub_block_size;
            ++n;

            // Move the header index to the header of the next block
            sub_block_header_index += sub_block_size + 1;
            sub_block_size = binary_content[sub_block_header_index];
        }
        std::cout << "\tFound terminator sub-block after " 
                  << n << " data blocks holding " << total_size 
                  << " lzw-encoded bytes" << std::endl;
        
        // There were total_size content bytes, n+1 block header bytes, and the lzw code size
        total_image_data_size = total_size + n + 2;

        return true;
    }

    bool verify_trailer() {
        std::cout << "Trailer: " << std::endl;

        auto index = COLOR_TABLE_OFFSET + local_color_table_bytes + total_image_data_size;
        if (binary_content.size() != index + 1) {
            error() << "Data size is inconsistent with computed offset. There are " 
                    << binary_content.size() 
                    << " bytes of data, but the trailer should be at index " 
                    << index 
                    << std::endl; 
            return false;
        }
        if (binary_content.at(index) != 0x3B) {
            error() << fmt::format("Incorrect trailer value: {:#04x}. Expected 0X3B.") << std::endl;
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