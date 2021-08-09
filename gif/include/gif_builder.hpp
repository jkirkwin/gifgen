#ifndef GIF_BUILDER_HPP
#define GIF_BUILDER_HPP

#include <ostream>
#include "image_utils.hpp"
#include "gif_block_buffer.hpp"
#include "../palettize/include/palettize.hpp" // TODO Fix this. This is the only way I've been able to make the include work, but it's unacceptable.

namespace gif {

    // Constructs a GIF data stream from one or more still images.
    class gif_builder {
    public:

        // Creates a new GIF builder which will write its data to
        // the provided ostream out. The dimensions width and height
        // must be the same for all images that are added to the data
        // stream. 
        // Neither dimension may be 0 or so large that it cannot be 
        // expressed in a 16-bit unsigned integer.
        gif_builder(std::ostream& out, std::size_t width, std::size_t height); // TODO Add timing value here?
        
        // The builder is not copyable or moveable.
        gif_builder(const gif_builder&) = delete;
        gif_builder& operator=(const gif_builder&) = delete;
        gif_builder(gif_builder&&) = delete;
        gif_builder& operator=(gif_builder&&) = delete;


        // Calls complete_stream() if the stream has not already been 
        // terminated.
        ~gif_builder();
        
        // Adds a still frame to the data stream, represented as a 
        // boost::gil Image View. This image must have the same 
        // dimensions as those given to the builder at construction. 
        //
        // Returns a reference to the builder for convenience.
        gif_builder& add_frame(const image::rgb_image_view_t& image_view);

        // Writes any buffered content to the output stream and 
        // terminates it as specified in the GIF standard. After
        // calling this function, all other non-const member functions
        // must not be called.
        void complete_stream();

    private:
        std::ostream& out_file;
        gif_block_buffer block_buffer;
        std::size_t width;
        std::size_t height;
        bool stream_complete;

        // Each member function is responsible for writing a 
        // well-defined block, sub-block, or collection thereof
        // to the output stream.
        void write_gif_header();
        void write_screen_descriptor();
        void write_image_descriptor(
            const image::rgb_image_view_t&, 
            const palettize::color_table&
        );
        void write_local_color_table(const palettize::color_table&);
        void write_image_data(
            const image::rgb_image_view_t&, 
            const palettize::color_table&
        );
        void write_gif_trailer();
    };

}

#endif 
