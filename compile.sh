mkdir -p bin
emcc -s WASM=1 -O3 -s TOTAL_MEMORY=268435456 -D__linux__ \
  march.cc uv.cc cut.cc objectize.cc xatlas.cpp \
  xs/src/libslic3r/TriangleMesh.cpp xs/src/libslic3r/Polygon.cpp xs/src/libslic3r/ExPolygon.cpp \
  xs/src/libslic3r/Point.cpp xs/src/libslic3r/Multipoint.cpp \
  xs/src/libslic3r/TransformationMatrix.cpp \
  xs/src/libslic3r/ClipperUtils.cpp xs/src/libslic3r/Log.cpp \
  xs/src/admesh/connect.c xs/src/admesh/normals.c xs/src/admesh/shared.c \
  xs/src/admesh/stlinit.c xs/src/admesh/stl_io.c xs/src/admesh/util.c \
  xs/src/clipper.cpp \
  xs/src/poly2tri/common/shapes.cc \
  xs/src/poly2tri/sweep/sweep.cc xs/src/poly2tri/sweep/sweep_context.cc xs/src/poly2tri/sweep/cdt.cc xs/src/poly2tri/sweep/advancing_front.cc \
  -Ixs/src/libslic3r -Ixs/src -Isrc/standalone -I/usr/local/Cellar/boost/1.72.0/include \
  --pre-js prefix.js --post-js postfix.js \
  -o bin/mc.js

#  util.cc FastNoise.cpp noise.cc cachedNoise.cc compose.cc march.cc tssl.cc light.cc heightfield.cc flod.cc noiser.cc cull.cc objectize.cc \
#  --pre-js prefix.js --post-js postfix.js \
#  -o bin/objectize.js
