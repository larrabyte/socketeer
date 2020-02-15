# templatemake.sh
# made by the larrabyte himself
# cant make a makefile

EXEC="socketeer"

# compilers, mingw64 and gnu/linux
GNUCC="x86_64-pc-linux-gnu-gcc"
WINCC="x86_64-w64-mingw32-gcc"
TGTCC=""

# mingw64 gcc flags
WINCOMPILEFLAGS="-O3 -flto -march=native"
WININCLUDEFLAGS=""
WINLIBRARYFLAGS="-lws2_32"

# gnu gcc flags
GNUCOMPILEFLAGS="-O3 -flto -march=native"
GNUINCLUDEFLAGS=""
GNULIBRARYFLAGS=""

# do not change these :)
OBJLIST=""
CFLAGS=""
IFLAGS=""
LFLAGS=""
set -e

function stageone() {
    for file in src/* ; do
        if [ -d "$file" ] ; then
            continue
        fi

        fbase=$(basename "$file")
        fname="${fbase%.*}"

        ${TGTCC} ${CFLAGS} ${IFLAGS} -c ${file} -o obj/${fname}.o
        printf "Compiling %s...\n" "$file"
    done
}

function stagetwo() {
    for file in obj/* ; do
        OBJLIST="${OBJLIST}${file} "
    done

    printf "Linking object files...\n"
    ${TGTCC} ${CFLAGS} ${OBJLIST} -o bin/${EXEC} ${LFLAGS}
}

# argument parser
if [ "$1" == "win" ] ; then
    CFLAGS="${WINCOMPILEFLAGS}"
    LFLAGS="${WINLIBRARYFLAGS}"
    IFLAGS="${WININCLUDEFLAGS}"
    TGTCC="${WINCC}"
else
    CFLAGS="${GNUCOMPILEFLAGS}"
    LFLAGS="${GNULIBRARYFLAGS}"
    IFLAGS="${GNUINCLUDEFLAGS}"
    TGTCC="${GNUCC}"
fi

stageone
stagetwo
