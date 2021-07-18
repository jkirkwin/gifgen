#ifndef LZW_HPP
#define LZW_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <vector>

namespace lzw {

    // Implementation details for the LZW library. 
    // Not for external use.
    namespace internal {

        // Encapsulates the logic needed to convert the bitstream
        // produced by the LZW encoding process into a sequence of 
        // bytes.
        //
        // Variable-length bit sequences are written to the 
        // byte_buffer. Once one or more bytes of data have been 
        // written, they are forwarded downstream to an OutputStream.
        template <class OutputStream>
        class byte_buffer {
            public:
                // Create a new byte buffer that will write 
                // byte-aligned data to the given OutputStream.
                byte_buffer(OutputStream& out) : 
                    bits_in_buffer(0), 
                    buffer(0), 
                    out_stream(out) {}

                // Add the num_bits least significant bits of 
                // data to the buffer.
                void insert(std::size_t num_bits, uint16_t data); // TODO implement buffer insertion

                // Flush the current buffer contents to the 
                // output stream. If the number of dirty bits
                // is not a multiple of 8, the unused bits will 
                // be zeros.
                void flush(); // TODO implement

            private:
                std::size_t bits_in_buffer;
                uint32_t buffer; // TODO Might make sense for this to be smaller.
                OutputStream &out_stream;
        };
    }

    // A class which is used to encode a sequence of 8-bit values
    // into a GIF-compliant LZW-encoded byte-stream.
    //
    // LZW is a relatively simple encoding scheme which maintains 
    // a running dictionary of previously seen sequences of input
    // symbols. Each time a new symbol sequence is seen, it is added
    // to the dictionary and emitted as a set of literals or shorter
    // sequence encodings. Each subsequent time that sequence is seen,
    // it is encoded as a single index in the dictionary.
    //
    // When constructed, the encoder can be configured to use a 
    // specific number of bits to encode output symbols (the code size). 
    // As the dictionary grows during the encoding process, more bits
    // may be required to encode each symbol or sequence.
    //
    // Initially the encoder's dictionary includes codes for each of the 
    // 2^intial_code_size 'literal' values as well as two special codes:
    //
    //      1. The clear code is defined as 2^code_size. This signals that 
    //         the dictionary has been cleared of all contents and the 
    //         code size has been reset to the originally specified value.
    //      2. The End of Information (EOI) code indicates the end of the
    //         data stream for an image. It is defined as clear_code + 1.
    //         The EOI code will always be written to the output stream 
    //         once encoding is complete.
    //
    // The first "generated" code representing a sequence of literals will
    // be EOI + 1.
    //
    // The output codes are constrained to be at most 12 bits. The maximum
    // code value is 0xFFF.
    //
    // The OutputStream type must implement operator<< to accept encoded 
    // values byte-by-byte *as they are encoded*. 
    template<class OutputStream>
    class lzw_encoder {
    public:

        using input_symbol_type = uint8_t;

        // Maximum code size is 12 bits, so 16 bits is enough for all codes. 
        using code_type = uint16_t; 
        
        using size_type = std::size_t;

        // The maximal number of bits that may be used for an encoded
        // value.
        constexpr static size_type max_code_size() noexcept {
            return MAX_CODE_SIZE;
        }

        // Creates an lzw encoder with an empty dictionary and the
        // provided number of bits to be used to encode output data
        // to begin with. Encoded data will be written to the provided
        // OutputStream one byte at a time.
        // 
        // The number of bits required to encode output 
        // data may increase as the size of the dictionary grows.
        // The first code that is written will have size 
        // starting_bits+1 due to the extra clear and EOI codes 
        // discussed above.
        //
        // starting_bits should be at least as large as the number
        // of bits required to represent every pixel in the colour 
        // table, and must not be less than 3;
        lzw_encoder(size_type starting_bits, OutputStream& out) : 
                    starting_code_size(starting_bits), 
                    current_code_size(starting_bits + 1),
                    byte_buf(out) {
            assert (starting_bits >= 3);
            assert (starting_bits <= max_code_size());

            // TODO Initialize the dictionary to hold the first 2**starting_bits values

            next_code = eoi_code() + 1;

            // According to various sources online, the safest thing to 
            // do is to start with a clear code to guarantee the 
            // decompressor won't start with non-standard values in its
            // dictionary.
            // TODO Send the clear code with write_code(clear_code())

            // TODO implement and test constructor
        }

        // Destroys the lzw_encoder after flushing any unwritten data
        // to the output stream and sending the EOI marker.
        ~lzw_encoder() {
            // TODO implement and test destructor. Should just need to call flush.
        } 

        // Encodes the single value i. This operation may or may not
        // result in the downstream object receiving encoded data, 
        // depending on the state of the dictionary. 
        //
        // TODO need to document what happens if we run out of space 
        // in the dictionary.
        //     - could use the clear code or could just continue 
        //       encoding stuff using 12 bits and stop adding new 
        //       sequences to the dictionary.
        // TODO implement and test encode overloads and operator<<. Use write_code().
        void encode(input_symbol_type i);
        lzw_encoder& operator<<(input_symbol_type i); // TODO this may need to be defined outside the class?

        // Encodes a sequence of values represented by the begin 
        // and end input iterators. The result of dereferencing
        // an InputIterator must be an input_symbol_type.
        // See encode() for more information. 
        template <class InputIterator>
        void encode(InputIterator begin, InputIterator end);
        // TODO Try using std::for_each(begin, end, encode);
    
        // Returns the number of bits currently being used for 
        // encoded values encoding.
        size_type code_size() const noexcept {
            return current_code_size; // TODO test code_size
        } 

        // Returns the current clear code
        code_type clear_code() const noexcept {
            return 1 << (code_size() - 1); // TODO test clear_code
        } 

        // Returns the current End of Information code
        code_type eoi_code() const noexcept {
            return clear_code() + 1; // TODO test eoi_code
        }

        // Encodes and writes any buffered data to the output 
        // stream, followed by the EOI marker.
        void flush(); // TODO implement and test flush. Encode current data and then write the final byte 

    private:
        using symbol_string_type = std::vector<input_symbol_type>;
        using dict_type = std::map<symbol_string_type, code_type>; // TODO consider other options for this 

        const static size_type MAX_CODE_SIZE = 12;

        // Bit sizes
        size_type starting_code_size;
        size_type current_code_size;

        // The next code to be added to the dictionary
        code_type next_code;

        // Current sequence of matched symbols
        symbol_string_type symbol_buf;

        dict_type dict;

        // Holds partial bytes from previously encoded sequences.
        internal::byte_buffer<OutputStream> byte_buf;

        // Adds the code to the bit buffer and writes part or all of 
        // the buffer to the output stream.
        void write_code(code_type code); // TODO implement write_code
    };
}

#endif