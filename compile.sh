mkdir -p build/wasm
emcc --std=c++11 -s WASM=1 -O3 \
  objectize.cc util.cc compose.cc tssl.cc light.cc \
  -o build/wasm/objectize.js
