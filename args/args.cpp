#include "args.hpp"
#include <getopt.h>
#include <iostream>
#include <cassert>

namespace args {

    // TODO Add -d option

    void print_help() {
        std::cout 
            << "gifgen reads in one or more PNG or JPEG images and embeds them in a GIF file." 
            << std::endl
            << std::endl
            << "To run the program, use:" << std::endl
            << "\tgifgen [-p | -j] <input file 1> <input file 2> [...] -o <result file name> [-t <delay>]" 
            << std::endl
            << std::endl
            << "Options:"
            << std::endl
            << "\t-p, --png" << std::endl
            << "\t\tIndicates the input files are PNG images (not compatible with -j)" 
            << std::endl
            << std::endl
            << "\t-j, --jpeg" << std::endl
            << "\t\tIndicates the input files are JPEG images (not compatible with -p)"  
            << std::endl
            << std::endl
            << "\t-o, --output" 
            << std::endl
            << "\t\tThe name of the gif file to be created. " << std::endl
            << "\t\tIf this file already exists, it will be overwritten." 
            << std::endl
            << std::endl
            << "\t-t, --timing" 
            << std::endl
            << "\t\tThe timing delay to insert between frames, measured in milliseconds." << std::endl
            << "\t\tThis must be a value between 0 and " << MAX_DELAY_MS << ", inclusive, and must be a multiple of 10." << std::endl
            << "\t\tThe default value is 0." 
            << std::endl
            << std::endl
            << "\t-h, --help" 
            << std::endl
            << "\t\tShow this help message" 
            << std::endl
            << std::endl;
    }

    void print_usage() {
        std::cout 
            << "Usage: gifgen [--jpeg | --png] <input files list> -o <output_file> [-t <timing delay>]" << std::endl 
            << "Use gifgen --help for more information." << std::endl;
    }

    // Prints the error message and shuts down the application
    void error(std::string error_message) {
        std::cout << "Error: " << error_message << std::endl << std::endl;
        print_usage();

        std::exit(EXIT_FAILURE);
    }

    // Checks the constructed arguments to verify that they are well 
    // formed. If an error is detected, the program is terminated.
    void validate_parsed_args(const program_arguments& args) {
        if (args.file_type == UNSPECIFIED) {
            error("No file type flag was specified");
        }
        else if (args.input_files.empty()) {
            error("No input files were specified");
        }
        else if (args.output_file_name == "") {
            error("No output file was specified");
        }
    }

    program_arguments parse_arguments(int argc, char **argv) {
        program_arguments args;
        args.file_type = UNSPECIFIED;
        args.delay = 0;

        int ind = 0; // Unused but required for getopt_long
        int cur_opt; // The option currently being processed

        // Defines all the long options that we support and
        // their mappings to a char identifier.
        static struct option opts[] = {
            {"png",     no_argument,       0,  'p'},
            {"jpeg",    no_argument,       0,  'j'},
            {"output",  required_argument, 0,  'o'},
            {"timing",  required_argument, 0,  't'},
            {"help",    no_argument,       0,  'h'},
            {0,         0,                 0,  0  }
        };

        // Defines the short versions of the options and whether they 
        // take any values. A colon indicates am argument.
        const auto optstring = "pjo:t:h";

        // Iterate over all specified options and add them to args.
        while ((cur_opt = getopt_long(argc, argv, optstring, opts, &ind)) != -1) {
            switch (cur_opt) {
                case 'p':
                case 'j':
                    if (args.file_type != UNSPECIFIED) {
                        error("Duplicate file type flags");
                    }
                    else {
                        args.file_type = static_cast<input_file_type>(cur_opt);
                    }

                    break;

                case 'o':
                    if (args.output_file_name != "") {
                        error("Duplicate output file flag");
                    }
                    else {
                        args.output_file_name = optarg;
                    }
                    break;

                case 't':
                    try {
                        // std::stoi may throw out_of_range or invalid_argument exceptions 
                        // on failure.
                        int parsed_delay = std::stoi(optarg);
                        if (parsed_delay < 0 || static_cast<std::size_t>(parsed_delay) > MAX_DELAY_MS) {
                            error("Timing delay out of allowable range");
                        }
                        else if (parsed_delay % 10 != 0) {
                            error("Timing delay must be a multiple of 10");
                        }
                        else {
                            args.delay = parsed_delay / 10;
                            assert (args.delay <= MAX_DELAY_VALUE);
                        }

                        break;
                    }
                    catch(std::exception& e) {
                        error("Unable to convert timing delay to integer value");
                    }

                case 'h':
                    // If we see the help flag, stop the application immediately after printing
                    // out the help message.
                    print_help();
                    std::exit(EXIT_SUCCESS);

                case '?': 
                    // getopt failed to recognize the current option. There isn't much
                    // we can do in the way of error reporting here, but getopt will 
                    // print some information on which option was malformed.
                    error("Failed to parse options");
                    break;

                default: 
                    // getopt recognized the option, but it isn't valid.
                    error("Unable to process flag " + cur_opt);
                    break;
            }
        }

        // Add all the non-option arguments to the result. These are the input
        // files to convert to GIF format.
        while (optind < argc) {
            std::string arg = argv[optind];
            args.input_files.push_back(arg);
            ++optind;
        }

        validate_parsed_args(args);

        return args;
    }
}