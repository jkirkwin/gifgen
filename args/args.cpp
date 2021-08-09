#include "args.hpp"
#include <getopt.h>
#include <iostream>

namespace args {

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
            << "\t\tIndicates the input files are PNG images" 
            << std::endl
            << "\t-j, --jpeg" << std::endl
            << "\t\tIndicates the input files are JPEG images" 
            << std::endl
            << "\t-o, --output" 
            << std::endl
            << "\t\tThe name of the gif file to be created" 
            << std::endl
            << "\t-t, --timing" 
            << std::endl
            << "\t\tThe timing delay to insert between frames, measured in hundreths of a second." << std::endl
            << "\t\tThis must be a value between 0 and " << MAX_DELAY << ", inclusive. The default value is 0." 
            << std::endl
            << "\t-h, --help" 
            << std::endl
            << "\t\tShow this help message" << std::endl
            << std::endl;
    }

    void print_usage() {
        std::cout 
            << "Usage: gifgen [--jpeg | --png] <input files list> -o <output_file>" << std::endl 
            << "Use gifgen --help for more information." << std::endl;
    }

    bool validate_args(const program_arguments& args) {
        return args.file_type != UNSPECIFIED
            && args.input_files.size() >= 1
            && args.output_file_name != "";
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

        // Iterate over all specified options
        while ((cur_opt = getopt_long(argc, argv, optstring, opts, &ind)) != -1) {
            switch (cur_opt) {
                case 'p':
                case 'j':
                    // It is an error to specify multiple or duplicate 
                    // file switches
                    if (args.file_type != UNSPECIFIED) {
                        print_usage();
                        std::exit(EXIT_FAILURE);
                    }
                    args.file_type = static_cast<input_file_type>(cur_opt);
                    break;
                
                case 'o':
                    args.output_file_name = optarg;
                    break;

                case 't':
                    try {
                        // stoi may throw out_of_range or invalid_argument on failure
                        int parsed_delay = std::stoi(optarg);
                        if (parsed_delay < 0 || static_cast<std::size_t>(parsed_delay) > MAX_DELAY) {
                            throw std::out_of_range("Delay out of allowable range");
                        }

                        args.delay = parsed_delay;

                        break;
                    }
                    catch(...) {
                        print_help();
                        std::exit(EXIT_FAILURE);
                    }

                case 'h':
                    print_help();
                    std::exit(EXIT_SUCCESS);

                case '?': // getopt failed to recognize the current option. 
                default: // getopt recognized the option, but it isn't valid.
                    print_usage();
                    std::exit(EXIT_FAILURE);
            }
        }

        // Add all the non-option arguments to the result. These are the input
        // files to convert to GIF format.
        while (optind < argc) {
            std::string arg = argv[optind];
            args.input_files.push_back(arg);
            ++optind;
        }

        if (!validate_args(args)) {
            print_usage();
            std::exit(EXIT_FAILURE);
        }

        return args;
    }
}