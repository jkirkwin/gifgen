BACKGROUND.
    This software makes up the project deliverable for SENG475: Advanced 
    Programming Techniques for Robust, Efficient Computing at UVic.

    The chosen project is an image-sequence -> animated GIF converter. 
    The converter takes as input a sequence of PNG or JPEG images and
    some optional frame-rate information and generates an animated GIF
    containing palettized versions of each of the input frames.

DEPENDENCIES.
    To build and run this software, the following libraries are required:
        * Boost 
        * Boost::GIL
        * zlib 
        * libpng
        * libjpeg/libjpeg-turbo

    The author used the following versions of those libraries to develop the software:
        * Boost (with GIL)  v1.76 
        * zlib              v1.2.11
        * libpng            v1.6.37
        * libjpeg-turbo     v62

TOOLING.
    This software should be built with the provided build files. These require a GCC 
    installation which supports both C++17 and C++20, as both are used due to limitations
    in the Boost::GIL library which necessitates the use of C++17 for image input 
    functionality. 

    Use the following commands to build the software:

        cmake -H. -Bbuild
        cmake --build build
