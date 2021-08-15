#include "args.hpp"
#include <getopt.h>
#include <iostream>
#include <cassert>
#include <filesystem>
#include <algorithm>

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
            <<"\t-d, --directory" << std::endl
            << "\t\tIgnore positional input file arguments and use all files in the top level of the" << std::endl
            << "\t\tspecified directory as input frames. The full contents of the directory will be" << std::endl
            << "\t\tprocessed, excluding sub-directories, in alphabetical order."
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

    // Parsing logic for file type argument 
    void set_file_type(program_arguments& args, char flag) {
        assert (flag == 'j' || flag == 'p');

        args.file_type = 
            flag == 'j' 
            ? image::file_type::JPEG 
            : image::file_type::PNG; 
    }

    // Parsing logic for timing delay
    void set_timing_delay(program_arguments& args, const std::string& delay_string) {
        try {
            // std::stoi may throw out_of_range or invalid_argument exceptions 
            // on failure.
            int parsed_delay = std::stoi(delay_string);
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
        }
        catch(std::exception& e) {
            error("Unable to convert timing delay to integer value");
        }
    }

    // Enumerate the files in the top level of the given directory
    std::vector<std::string> enumerate_directory_files(const std::string& dir) {
        assert (std::filesystem::exists(dir));
        assert  (std::filesystem::is_directory(dir));

        std::vector<std::string> files;
        for(auto const& dir_entry : std::filesystem::directory_iterator{dir}) {
            if (dir_entry.is_regular_file()) {
                files.push_back(dir_entry.path());
            }
        }

        return files;     
    }

    // Add the contents of the directory to the arguments in alphabetical order, if possible.
    void add_dir_files_to_args(program_arguments& args, const std::string& dir) {
        assert (std::filesystem::exists(dir)); 
        assert (std::filesystem::is_directory(dir));

        auto files = enumerate_directory_files(dir);
        if (files.empty()) {
            error("No files were found in directory " + dir);
        }
        else {
            std::sort(files.begin(), files.end());
            args.input_files = files;
        }
    }

    // Parsing logic for input directory option
    void set_input_directory(program_arguments& args, const std::string& dir_name) {
        assert (args.input_files.empty());
        
        if (!std::filesystem::exists(dir_name) ||
                !std::filesystem::is_directory(dir_name)) {
            error ("No such directory: " + dir_name);
        }
        else {
            try {
                add_dir_files_to_args(args, dir_name);
            }
            catch(...) {
                // There are numerous things that might go wrong when
                // accessing the directory and its files. Leave it to 
                // the user to diagnose the issue in this case.
                error("Unable to parse directory " + dir_name);
            }
        }
    }

    program_arguments parse_arguments(int argc, char **argv) {
        program_arguments args;
        bool found_file_type = false;
        bool found_delay = false;
        args.delay = 0;

        int ind = 0; // Unused but required for getopt_long
        int cur_opt; // The option currently being processed

        // Defines all the long options that we support and
        // their mappings to a char identifier.
        static struct option opts[] = {
            {"png",         no_argument,       0,  'p'},
            {"jpeg",        no_argument,       0,  'j'},
            {"output",      required_argument, 0,  'o'},
            {"timing",      required_argument, 0,  't'},
            {"directory",   required_argument, 0,  'd'},
            {"help",        no_argument,       0,  'h'},
            {0,             0,                 0,  0  }
        };

        // Defines the short versions of the options and whether they 
        // take any values. A colon indicates an argument.
        const auto optstring = "pjo:t:d:h";

        // Iterate over all specified options and add them to args.
        while ((cur_opt = getopt_long(argc, argv, optstring, opts, &ind)) != -1) {
            switch (cur_opt) {
                case 'p':
                case 'j':
                    if (found_file_type) {
                        error("Duplicate file type flags");
                    }
                    else {
                        set_file_type(args, cur_opt);
                        found_file_type = true;
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
                    if (found_delay) {
                        error("Duplicate timing delay specified");
                    }
                    else {
                        set_timing_delay(args, optarg);
                        found_delay = true;
                    }

                    break;

                case 'd':
                    if (!args.input_files.empty()) {
                        error("Cannot source input files from multiple directories");
                    }
                    else {
                        set_input_directory(args, optarg);
                    }

                    break;

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

        // Check for missing values
        if (!found_file_type) {
            error("No file type flag was specified");
        }
        else if (args.output_file_name == "") {
            error("No output file was specified");
        }

        // If an input directory wasn't specified, add all the
        // non-option arguments to the result as input files.
        // Afterwards, ensure we've got at least one usable input 
        // file.
        bool dir_specified = !args.input_files.empty();
        while (optind < argc) {
            std::string arg = argv[optind];

            if (dir_specified) {
                std::cout << "Warning: Unused argument " << arg << ". The specified directory is used for input data instead." << std::endl;
            }
            else {
                args.input_files.push_back(arg);
            }
            
            ++optind;
        }

        if (args.input_files.empty()) {
            error("No input files were specified");
        }

        return args;
    }
}