emcc --std=c++11 -s WASM=1 -O3 \
  objectize.cc compose.cc tssl.cc \
  -o objectize.js
