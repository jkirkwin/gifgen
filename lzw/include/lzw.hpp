#ifndef LZW_HPP
#define LZW_HPP

#include <cstdint>

namespace lzw {

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

        // The maximum code size is 12 bits, so a 16 bit code representation
        // is sufficient. 
        using code_type = uint16_t; 
        
        using size_type = std::size_t;

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
        // table.
        lzw_encoder(size_type starting_bits, OutputStream& out); // TODO implement and test constructor

        // Destroys the lzw_encoder after flushing any unwritten data
        // to the output stream and sending the EOI marker.
        ~lzw_encoder(); // TODO implement and test destructor

        // Encodes the single value i. This operation may or may not
        // result in the downstream object receiving encoded data, 
        // depending on the state of the dictionary. 
        //
        // TODO need to document what happens if we run out of space 
        // in the dictionary.
        //     - could use the clear code or could just continue 
        //       encoding stuff using 12 bits and stop adding new 
        //       sequences to the dictionary.
        // TODO implement and test encode overloads and operator<<
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
        size_type code_size() const; // TODO implement and test code_size

        // Returns the current clear code
        code_type clear_code() const; // TODO implement and test clear_code

        // Returns the current End of Information code
        code_type eoi_code() const; // TODO implement and test eoi_code

        // Encodes and writes any buffered data to the output 
        // stream, followed by the EOI marker.
        void flush(); // TODO implement and test flush

    private:
        size_type starting_code_size;
        size_type current_code_size;

        // TODO Need a vector or similar to hold current buffer 
        // of input symbols

        // TODO Need a dictionary implementation. 
        //  - Keys are sequences of bytes.
        //  - Values are code_type's
        //  - The maximum number of entries is 4096 (12 bits max for code sizes)
        // To start we could probably use strings or vector<uint8_t> as keys in a
        // map or unordered_map
        // Map might be okay because we'd be unlikely to have lots of patterns of the same
        // length with long, common prefixes. Vector provides lexicographic comparison for us.
    };
}

#endif