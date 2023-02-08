emcc xatlas.cpp -o xatlas.js -std=c++11 -O3 --closure 1 -s ENVIRONMENT='shell' -s WASM_OBJECT_FILES=0 --llvm-lto 1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ALLOW_MEMORY_GROWTH=1 -fno-rtti -fno-exceptions
