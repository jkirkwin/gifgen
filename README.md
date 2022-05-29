# `gifgen` GIF Generator

`gifgen` is a GIF generation tool which embeds multiple still-frame PNG or JPEG images into an animated GIF image. It was developed as part of Dr Adams' course on Advanced Programming Techniques for Robust, Efficient Computing at UVIC. A video demonstration created for the course is available on [YouTube](https://www.youtube.com/watch?v=7rIRGN_ikXw).

## Background

`gifgen` is a a command-line application which converts sequences of still images into animated GIFs. The converter takes as input a sequence of PNG or JPEG images and optional frame-rate information and generates as output an animated GIF containing palettized versions of each of the input frames.

GIF supports only an 8-bit color space, so images that use more than 256 unique colors must be quantized in some manner to decrease their color variety. This is done using a slightly modified version of the classical Median Cut algorithm, as presented by Burger and Burge in [Principals of Digital Image Processing: Core Algorithms](https://www.springer.com/gp/book/9781848001947). The process of generating a palette of 256 or fewer colors for an image and quantizing the image pixels to use only those colors is referred to hereafter and in the code as "palettization".

Once a palette has been generated, each image is re-encoded as a sequence of indices into the list of colors which form that color palette. Then, the sequence of indices is encoded using LZW compression. The LZW algorithm prescribed by the GIF specification is slightly modified from the orgiginal to include a clear code, and end-of-stream marker, and variable size code words.

Each input frame is palettized, encoded using LZW, and inserted into a formatted GIF-compliant data stream. Other information can also be embedded in this data stream, including animation directives. The software supports a custom timing delay value; this value is measured in hundreths of a second and will cause a GIF-compliant image viewer to pause on each frame for the specified period of time before loading the next one. This effectively allows the user to specify a frame-rate for multi-frame GIF animations.

## Display Software
Different image viewing programs can display GIF files, but they do not all behave the same way. Notably, the Unix display tool and ffplay (from the ffmpeg toolsuite) do not conform strictly to the GIF89a standard's specifications on animation and do not properly support the ubiquitous NETSCAPE2.0 control extension which specifies looping behaviour. As such, it is recommended that these tools should not be used to view the resulting GIFs. An up-to-date web browser is a safer choice.

## Dependencies
To build and run this software, the following libraries are required:
* Boost 
* Boost::GIL
* zlib 
* libpng
* libjpeg/libjpeg-turbo

The author used the following versions of those libraries to develop the software:
| Package       | Version |
|---------------|---------|
| Boost         | v1.76   |
| zlib          | v1.2.11 |
| libpng        | v1.6.37 |
| libjpeg-turbo | v62     |

Other tools like lcov, cmake, g++, and catch2 are also needed.

Installing these tools can be difficult depending on your system. See the __Docker__ section below for an easier option.

### A note on gcc/g++
To build this project, a GCC installation which which supports both C++17 and C++20 is required. C++20 is used wherever possible, but Boost::GIL's image input/output functionality is not (at the time of writing) compatible with C++20, so C++17 is used to build the component(s) which rely on it.

## Usage

Use the following commands to build the software with coverage and testing targets enabled:
``` bash
    cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=DEBUG -DENABLE_COVERAGE=ON 
    cmake --build build
```

Use the following commands to build the software for actual use:
``` bash
    cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE
    cmake --build build --target gifgen
```

## Installation

The `install` target can be used to build and install the software. The gifgen binary is placed in the directory `${CMAKE_INSTALL_PREFIX}/bin` during install. To install the software in a custom location, specify a custom value for CMAKE_INSTALL_PREFIX. For example:
``` bash
    cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local
    cmake --build build --target install
    /usr/local/bin/gifgen <some args>
```

As noted again below, if the software is being installed, the user should specify the RELEASE configuration unless they have a good reason not to. The software can be installed using a different configuration like DEBUG, but it will be far less performant.

## Docker

The above installation/running instructions were originally written for the marker of this final project. For other users, a simpler way to build and run the application is provided via Docker.

You can pull an image containing a pre-tested and pre-installed version of `gifgen` with `docker pull jkirkwin/gifgen:latest`.

To create a container from your image, you might use something like the following:
```
docker run -it jkirkwin/gifgen:latest
```

From the shell in the docker container you can run `gifgen` however you like, but you may need to copy files or add volumes if you want to use your own files as inputs.

## Performance
Since Catch2 and LCov are used for unit testing and code coverage generation respectively, building unit tests and/or building with coverage enabled will drastically slow down the build process. Catch2 incurs a lot of overhead due to repeated use of the large single header file.
* To build the software without coverage enabled, set the ENABLE_COVERAGE flag to OFF.
* To build the gifgen application without building the unit tests, use the `gifgen` target directly.

Non-RELEASE builds should *only* be used for testing or for generating GIFs from trivial inputs. The optimizations included in the release configuration will speed up the application by *at least* an order of magnitude. Especially for large, high resolution images, a debug version of the application will be prohibitively slow.
