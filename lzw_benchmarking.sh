# A helper script to evaluate possible LZW encoder
# optimizations 

ITERATIONS=3

BUILD_DIR=benchmarking
cmake -H. -B${BUILD_DIR} -DCMAKE_BUILD_TYPE=RELEASE
cmake --build ${BUILD_DIR} --target gifgen

SEPARATOR="\n======================\n"

echo -e $SEPARATOR

function run_command_n_times() {
    GIFGEN_CMD=$1
    
    for (( i=1; i<=$ITERATIONS; i++ )); do
        echo -e "\nIteration $i"
        time $GIFGEN_CMD > /dev/null
    done
}

echo "Snake: "
run_command_n_times "./${BUILD_DIR}/gifgen --png -d test_images/snake -o lzw_snake.gif -t 500"

echo -e $SEPARATOR

echo "Fishing: "
run_command_n_times "./${BUILD_DIR}/gifgen --jpeg test_images/jpeg_fishing.jpg -o lzw_fishing.gif"

echo -e $SEPARATOR

echo "Skydiving: "
run_command_n_times "./${BUILD_DIR}/gifgen --jpeg -d test_images/skydiving -o lzw_skydiving.gif"

echo -e $SEPARATOR
