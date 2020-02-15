# make.sh because i can't write a makefile
# written by the larrabyte himself

COMPILERFLAGS="-static -O3 -flto -march=native" # "-Wall -Wextra -Wpedantic -Wno-format"
COMPILER="x86_64-w64-mingw32-gcc" # gcc

LIBRARIES="-lws2_32"
EXECNAME="socketeer"
OBJFILELIST=""

set -e

# Cleanup code
for arg in "$@" ; do
    if [ "$arg" == "clean" ] ; then
        printf "Cleaning ./obj/ of object files...\n"
        find ./obj -type f -name "*.o" -delete
        exit 0
    fi
done

# stage 1: source -> object
for file in src/* ; do

    # if a directory, skip
    if [ -d "$file" ] ; then
        continue
    fi

    # grab name w/out extension
    filebase=$(basename "$file")
    filename="${filebase%.*}"

    # compile and print
    ${COMPILER} ${COMPILERFLAGS} -c -o obj/${filename}.o ${file}
    printf "Compiling %s...\n" "$file"
done

# stage 2: get all files in ./obj
for file in obj/* ; do
    OBJFILELIST="${OBJFILELIST}${file} "
done

# stage 3: link -> executable
printf "Linking object files...\n"
${COMPILER} ${COMPILERFLAGS} -o bin/${EXECNAME} ${OBJFILELIST} ${LIBRARIES}
