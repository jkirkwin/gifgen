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
        //
        // Bits from a code word are packed into bytes from least to 
        // most significant bit, starting a new byte whenever 8 bits
        // are collected.
        template <class OutputStream>
        class byte_buffer {
            public:

                // Create a new byte buffer that will write 
                // byte-aligned data to the given OutputStream.
                byte_buffer(OutputStream& out) : 
                    bits_in_buffer(0), 
                    buffer(0), 
                    out_stream(out), 
                    flush_started(false) {}

                // Destroy the byte buffer after flushing all 
                // buffered data downstream.
                ~byte_buffer() {
                    if (!flush_started) {
                        flush();
                    }
                }

                // Add the num_bits least significant bits of 
                // data to the buffer.
                void insert(std::size_t num_bits, uint16_t data) {
                    assert (!flush_started);
                    assert (num_bits <= 16);

                    // Empty out the buffer as much as we can
                    while (bits_in_buffer >= 8) {
                        write_byte();
                    }

                    // Isolate the lower num_bits of the provided data
                    if (num_bits < 16) {
                        data &= get_n_ones(num_bits);
                    }

                    // Insert the data to the right of any existing 
                    // buffer contents
                    assert (num_bits + bits_in_buffer <= 32);
                    buffer |= (data << bits_in_buffer);
                    bits_in_buffer += num_bits;
                }

                // Flush the current buffer contents to the 
                // output stream. If the number of dirty bits
                // is not a multiple of 8, the unused bits will 
                // be zeros. 
                // flush should only be called once, and no other 
                // non-const member functions may be called after
                // invoking flush.
                void flush() {
                    assert (!flush_started);
                    flush_started = true;

                    while(has_data()) {
                        write_byte();
                    }
                }

            private:
                // Since only 16 bits may be inserted at a time, 32 bits
                // is a safe choice for the internal buffer size to prevent
                // the possibility of bits being lost due to shift operations.
                using int_buffer_type = uint32_t;

                // Tracks buffer contents
                std::size_t bits_in_buffer;
                int_buffer_type buffer;

                OutputStream &out_stream;

                bool flush_started;
        
                bool has_data() const noexcept {
                    return bits_in_buffer > 0;
                }

                void write_byte() {
                    assert (bits_in_buffer >= 8 || flush_started);
                    assert (bits_in_buffer > 0);

                    // Write the least significant 8 bits downstream and
                    // remove them from the buffer.
                    uint8_t lsb(buffer & 0xFF);
                    out_stream << lsb;
                    buffer = (buffer >> 8);

                    // Account for possible overflow when updating the 
                    // remaining bits in case we're in the process of flushing.
                    if (bits_in_buffer >= 8) {
                        bits_in_buffer -= 8;
                    }
                    else {
                        bits_in_buffer = 0;
                    }
                }

                // Creates a bitmask with n 1's in it, starting from 
                // the LSbit 
                uint16_t get_n_ones(std::size_t n) {
                    return ((1u << n) - 1u);
                }
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
        // to begin with. This value must be such that all the 
        // literals to be encoded can fit in symbols of that many bits.
        //
        // Encoded data will be written to the provided OutputStream 
        // one byte at a time.
        // 
        // The number of bits required to encode output data may 
        // increase as the size of the dictionary grows.
        // The first code that is written will have size 
        // starting_bits + 1.
        lzw_encoder(size_type starting_bits, OutputStream& out) : 
                    starting_code_size(starting_bits), 
                    current_code_size(starting_bits + 1),
                    flushed(false),
                    byte_buf(out) {
            assert (starting_bits >= 3);
            assert (starting_bits <= 8);

            // Initialize the dictionary and send a clear code to
            // start the data stream.
            clear();
        }

        // Destroys the lzw_encoder after flushing any unwritten data
        // to the output stream and sending the EOI marker.
        ~lzw_encoder() {
            if (!flushed) {
                flush();
            }
        } 

        // Encodes the single value i. This operation may or may not
        // result in the downstream object receiving encoded data, 
        // depending on the state of the dictionary. 
        //
        // In the event that the internal dictionary becomes full,
        // a clear code will be emitted and the dictionary will be 
        // rebuilt.
        void encode(input_symbol_type i) {
            // Add the provided symbol to the current matched sequence
            symbol_string_type augmented = symbol_buf;
            augmented.push_back(i);

            auto it = dict.find(augmented);
            if (it != dict.end()) {
                // Augmented string has been seen before. Use
                // swap to add i to the buffer without risking
                // possible double re-allocation of memory from 
                // duplicate push_back calls. 
                std::swap(symbol_buf, augmented);
            }
            else {
                // Write the code for the currently matched string
                // and make the new working string the most recently
                // matched character.
                encode_buffered_symbols();
                symbol_buf.push_back(i);
                assert (symbol_buf.size() == 1);

                // If the dictionary is full, clear it. Otherwise, we add 
                // the augmented string to the dictionary. 
                if (next_code > MAX_CODE_VALUE) {                  
                    clear();
                }
                else {
                    add_code_for_string(augmented);
                }
            }
        }

        // A convenience wrapper around encode(). See
        // above for details.
        lzw_encoder& operator<<(input_symbol_type i) {
            encode(i);
            return *this;
        }

        // Encodes a sequence of values represented by the begin 
        // and end input iterators. The result of dereferencing
        // an InputIterator must be an input_symbol_type.
        // See encode() for more information. 
        template <class InputIterator>
        void encode(InputIterator begin, InputIterator end) {
            while (begin != end) {
                encode(*begin);
                ++begin;
            }
        }
    
        // Returns the number of bits currently being used for 
        // encoded values encoding.
        size_type code_size() const noexcept {
            return current_code_size;
        } 

        // Returns the current clear code
        code_type clear_code() const noexcept {
            return 1 << starting_code_size;
        } 

        // Returns the current End of Information code
        code_type eoi_code() const noexcept {
            return clear_code() + 1;
        }

        // Encodes and writes any buffered data to the output 
        // stream, followed by the EOI marker. 
        // Flush should only be called once. After flushing, 
        // the encoder may not be used to encode further data.
        void flush() {
            assert (!flushed);
            flushed = true;

            // Write any existing symbols which have a corresponding
            // dictionary entry, followed by the EOI stream terminator.
            encode_buffered_symbols();
            write_code(eoi_code());

            byte_buf.flush();
        }

    private:
        using symbol_string_type = std::vector<input_symbol_type>;
        using dict_type = std::map<symbol_string_type, code_type>; // TODO consider other options for this. A prefix tree would be much more space-efficient

        const static size_type MAX_CODE_SIZE = 12;
        const static size_type MAX_CODE_VALUE = 4095;

        // Bit sizes
        size_type starting_code_size;
        size_type current_code_size;

        // The next code to be added to the dictionary. 
        // Use a 32-bit value to prevent wrapping back to 0 when
        // the dictionary gets full.
        uint32_t next_code;

        // Current sequence of matched symbols
        symbol_string_type symbol_buf;
        bool flushed;

        dict_type dict;

        // Holds partial bytes from previously encoded sequences
        // and writes data downstream.
        internal::byte_buffer<OutputStream> byte_buf;

        // Adds the code to the bit buffer. May cause data to be written
        // downstream.
        void write_code(code_type code) {
            byte_buf.insert(current_code_size, code);
        } 

        // Encodes the currently buffered symbols and writes the encoding
        // downstream. Clears the symbol buffer. Does not change the state
        // of the dictionary.
        void encode_buffered_symbols() {
            if (!symbol_buf.empty()) {
                auto code_entry = dict.find(symbol_buf);
                write_code(code_entry->second);
                symbol_buf.clear();
            }
        }

        // Inserts the clear code into the output stream
        // and resets the dictionary to its original state.
        void clear() {
            // Write any buffered symbols downstream before sending 
            // the clear code.
            encode_buffered_symbols();
            write_code(clear_code());

            dict.clear();

            // Initialize the dictionary to hold mappings for all 
            // the literal values [0, 2^starting_bits).
            uint16_t max_literal(clear_code() - 1);
            for (uint16_t i = 0; i <= max_literal; ++i)  {
                input_symbol_type literal(i);
                symbol_string_type key {literal};
                code_type value(i);

                // This assertion structure is a little ugly, but lets us
                // avoid unused variable errors/warnings when asserts are
                // disabled.
#ifdef NDEBUG
                dict.insert(std::make_pair(key, value));
#else
                auto result = dict.insert(std::make_pair(key, value));
                assert (result.second);
#endif
            }

            // Reset our starting point in the code list to the 
            // first empty slot following the EOI code. 
            next_code = eoi_code() + 1;
            current_code_size = starting_code_size + 1;
        }

        // Add the given string of symbols to the dictionary
        // and update next_code and possibly the current code size.
        void add_code_for_string(const symbol_string_type& s) {
            assert (!dict.contains(s));
            assert (next_code <= MAX_CODE_VALUE);

            auto mapping = std::make_pair(s, next_code);
            dict.insert(mapping);
            ++next_code;

            // If we've hit the maximum code we can represent with the
            // current number of bits, increase the code size.
            // We don't need to worry about overflowing the allowed dict 
            // size/max code size here.
            if ((1u << current_code_size) < next_code) {
                ++current_code_size;
                assert ((1u << current_code_size) > next_code);
            }
        }
    };
}

#endif