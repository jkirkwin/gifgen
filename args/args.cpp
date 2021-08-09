#include "args.hpp"
#include <getopt.h>
#include <iostream>

namespace args {

    void print_help() {
        // TODO Update the help message
        std::cout 
            << "gifgen reads in one or more PNG or JPEG images and embeds them in a GIF file." << std::endl
            << std::endl
            << "To run the program, use:" << std::endl
            << "\tgifgen <filetype option> <input file 1> <input file 2> [...] -o <result file name>" << std::endl
            << "where <filetype option> is one of the following values:" << std::endl
            << "\t-p, --png" << std::endl
            << "\t\tIndicates the input files are PNG images" << std::endl
            << "\t-j, --jpeg" << std::endl
            << "\t\tIndicates the input files are JPEG images" << std::endl
            << std::endl
            << "Other options:" << std::endl
            << "\t-o, --output" << std::endl
            << "\t\tThe name of the gif file to be created" << std::endl
            << "\t-h, --help" << std::endl
            << "\t\tShow this help message" << std::endl
            << std::endl;
    }

    void print_usage() {
        std::cout 
            << "Usage: gifgen <filetype option> <input files list>" << std::endl 
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

        int ind = 0; // Unused but required for getopt_long
        int cur_opt; // The option currently being processed

        // Defines all the long options that we support and
        // their mappings to a char identifier.
        static struct option opts[] = {
            {"png",     no_argument,       0,  'p'},
            {"jpeg",    no_argument,       0,  'j'},
            {"output",  required_argument, 0,  'o'},
            {"help",    no_argument,       0,  'h'},
            {0,         0,                 0,  0  }
        };

        // Defines the short versions of the options and whether they 
        // take any values. A colon indicates am argument.
        const auto optstring = "pjo:h";

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