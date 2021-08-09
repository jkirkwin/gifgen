# A helper script which runs gifgen on all the test files 


USAGE="Usage: run_all.sh [pattern]"
PATTERN=""

if [ $# -gt 1 ]; then
    echo $USAGE
    exit 1
elif [ $# -gt 0 ]; then
    PATTERN=$1
fi

OUTPUT_DIR="results"

mkdir -p $OUTPUT_DIR

function run_gifgen() {
    IN_FILE=$1

    # Decompose the path into its base (no directories)
    # file name and extension. Use the file name to generate
    # an appropriate output file name and use the file extension
    # to determine the file type.
    IN_FILE_BASE=$(basename $IN_FILE)
    IN_FILE_EXT="${IN_FILE_BASE##*.}"
    IN_FILE_BASE=${IN_FILE_BASE%.*}
 
    FILE_TYPE_OPTION=$IN_FILE_EXT
    if [ "$FILE_TYPE_OPTION" = "jpg" ]; then 
        FILE_TYPE_OPTION="jpeg"
    fi

    OUT_FILE="${OUTPUT_DIR}/${IN_FILE_BASE}.gif"

    ./build/gifgen --$FILE_TYPE_OPTION -f $IN_FILE -o $OUT_FILE

    # TODO Run degif too?
}

if [ ! -d "./build" ]; then
    cmake -H. -Bbuild
fi

cmake --build build

TEST_IMAGES_DIR="../test_images"

MATCHING_FILES=""
if [[ -z "$PATTERN" ]]; then
    echo "No pattern given. Running all test files."
    MATCHING_FILES=$(find $TEST_IMAGES_DIR -type f \( -iname \*.jpg -o -iname \*.jpeg -o -iname \*.png \))
else 
    MATCHING_FILES=$(find $TEST_IMAGES_DIR -type f | grep "$PATTERN")
fi

for f in $MATCHING_FILES; do
    echo -e "\n$f"
    run_gifgen $f
done
