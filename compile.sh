mkdir -p bin
emcc --std=c++11 -s WASM=1 -O3 -s TOTAL_MEMORY=134217728 \
  util.cc FastNoise.cpp noise.cc cachedNoise.cc compose.cc march.cc tssl.cc light.cc heightfield.cc flod.cc noiser.cc cull.cc objectize.cc \
  -o bin/objectize.js
