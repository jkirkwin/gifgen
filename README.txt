BACKGROUND.
    This software makes up the project deliverable for SENG475: Advanced 
    Programming Techniques for Robust, Efficient Computing at UVic.

    The chosen project is an image-sequence -> animated GIF converter. 
    The converter takes as input a sequence of PNG or JPEG images and
    optional frame-rate information and generates as output an animated
    GIF containing palettized versions of each of the input frames.

    GIF supports only an 8-bit color space, so images that use more than 
    256 unique colors must be quantized in some manner to decrease their 
    color space to the appropriate size. This is done using a slightly 
    modified version of the classical Median Cut algorithm as presented
    by Burger and Burge in Princeipals of Digital Image Processing:
    Core Algorithms. The process of generating a palette of 256 or fewer
    colors for an image and quantizing it to use only those colors is 
    referred to hereafter and in the code as "palettization".

    Once a palette has been generated, each image is re-encoded as a sequence
    of indices into the color palette, and is then encoded using LZW compression.
    The LZW algorithm prescribed by the GIF specification is slightly modified
    to include clear and end-of-stream markers, and includes variable size codes
    to save space. 

    Each frame is palettized, encoded using LZW, and inserted into a formatted
    GIF-compliant data stream. Other information can also be embedded in this 
    data stream, including animation directives. The software supports a custom
    timing delay value; this value is measured in hundreths of a second and will
    cause a GIF-compliant image viewer to pause on each frame for that period of 
    time. This effectively allows the user to specify a frame-rate for multi-frame
    GIF animations.

DEPENDENCIES.
    To build and run this software, the following libraries are required:
        * Boost 
        * Boost::GIL
        * zlib 
        * libpng
        * libjpeg/libjpeg-turbo

    The author used the following versions of those libraries to develop 
    the software:
        * Boost           v1.76 
        * zlib            v1.2.11
        * libpng          v1.6.37
        * libjpeg-turbo   v62

TOOLING.
    To build this project, a GCC installation which which supports both C++17
    and C++20 is required. C++20 is used wherever possible, but Boost::GIL's 
    image input/output functionality is not (at the time of writing) compatible 
    with C++20, so C++17 is used to build the component(s) which rely on it.

USAGE.

    Use the following commands to build the software with coverage and testing 
    targets enabled:

        cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=DEBUG -DENABLE_COVERAGE=ON 
        cmake --build build

    Use the following commands to build the software for actual use:

        cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE
        cmake --build build --target gifgen

INSTALLATION.

    The install target can be used to build and install the software. The gifgen
    binary and a demo script with some sample image files are placed in the
    directory ${CMAKE_INSTALL_PREFIX}/bin. To install the software in a custom
    location, specify a custom value for CMAKE_INSTALL_PREFIX. For example, to 
    install a release version of the software in $TMP_DIR and run the demo script,
    the following commands might be used:

        cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=$TMP_DIR
        cmake --build build --target install
        $TMP_DIR/bin/demo 

    As noted again below, if the software is being installed, the user should 
    specify the RELEASE configuration unless they have a good reason not to. The 
    software can be installed using a different configuration like DEBUG, but it 
    will be far less performant.

PERFORMANCE.

    Imporant notes on DEBUG vs. RELEASE configuration performance for the marker
    and any prospective users:
    
    1.  Since Catch2 and LCov are used for unit testing and code coverage 
        generation respectively, building unit tests and/or building with coverage
        enabled will drastically slow down the build process. Catch2 incurs a lot
        of overhead due to repeated use of the large single header file.

        To build the software without coverage enabled, set the ENABLE_COVERAGE flag
        to OFF.

        To build the gifgen application without building the unit tests, use the 
        `gifgen` target directly.

    2.  Non-RELEASE builds should *only* be used for testing or for generating 
        GIFs from trivial inputs. The optimizations included in the release 
        configuration will speed up the application by *at least* an order of 
        magnitude. Especially for large, high resolution images, a debug version
        of the application will be prohibitively slow.
