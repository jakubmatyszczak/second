rm second
bear --output compile_commands.json -- \
    g++ src/main.cpp raylib/linux/lib/libraylib.a -Iraylib/include -O0 -g3 -o second -Wmissing-field-initializers 
./second
