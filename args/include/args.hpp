#ifndef ARGS_HPP
#define ARGS_HPP

#include <string>
#include <vector>

// Enables the parsing of command-line arguments into
// simple structures.
namespace args {

    // The maximal delay allowed in a GIF Graphics
    // Control Extension block, measured in hundredths
    // of a second.
    constexpr std::size_t MAX_DELAY_VALUE = 0xFFFF; 

    // The maximal delay allowed in a GIF Graphics
    // Control Extension block, measured in milliseconds.
    constexpr std::size_t MAX_DELAY_MS = MAX_DELAY_VALUE * 10;

    enum input_file_type : char {
        PNG = 'p',
        JPEG = 'j', 
        UNSPECIFIED = 0
    };

    // Represents the parsed command-line arguments required
    // to run the program
    struct program_arguments {
        input_file_type file_type;
        std::vector<std::string> input_files;
        std::string output_file_name;
        std::size_t delay;
    };

    // Parses the command-line arguments into a program_arguments
    // struct. std::exit will be invoked as appropriate if 
    //  (a) an invalid set of arguments is provided, or
    //  (b) the help option is specified
    program_arguments parse_arguments(int argc, char **argv);
}

#endif