FROM debian:stable

COPY . /usr/src

RUN apt-get update && \ 
    apt-get upgrade -y && \
    apt-get install -y g++ cmake libboost-all-dev git libjpeg62-turbo-dev libpng-dev lcov

WORKDIR /usr/src

# Set up catch2
RUN git clone https://github.com/catchorg/Catch2.git
WORKDIR /usr/src/Catch2
RUN git checkout v2.x
RUN cmake -Bbuild -H. -DBUILD_TESTING=OFF
RUN cmake --build build/ --target install

WORKDIR /usr/src

# Build and run the tests
RUN cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=DEBUG -DENABLE_COVERAGE=ON 
RUN cmake --build build
RUN ./build/gif/test_gif_block_buffer 
RUN ./build/palettize/test_color_table 
RUN ./build/palettize/test_median_cut 
RUN ./build/lzw/test_lzw 

# Rebuild in release mode and install to /usr/local/bin
RUN cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local
RUN cmake --build build --target install

CMD bash
