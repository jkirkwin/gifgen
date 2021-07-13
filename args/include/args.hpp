#ifndef ARGS_HPP
#define ARGS_HPP

#include <string>
#include <vector>

// Enables the parsing of command-line arguments into
// simple structures.
namespace args {

    enum input_file_type : char {
        PNG = 'p',
        JPEG = 'j', 
        UNSPECIFIED = 0
    };

    // Represents the parsed command-line arguments required
    // to run the program
    struct program_arguments {
        input_file_type file_type;
        std::vector<std::string> other_args;
    };

    // Parses the command-line arguments into a program_arguments
    // struct. std::exit will be invoked as appropriate if 
    //  (a) an invalid set of arguments is provided, or
    //  (b) the help option is specified
    program_arguments parse_arguments(int argc, char **argv);
}

#endif