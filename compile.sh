mkdir -p bin
emcc -s WASM=1 -O3 -s TOTAL_MEMORY=268435456 -D__linux__ \
  march.cc uv.cc cut.cc objectize.cc xatlas.cpp cmprs.cc \
  xs/src/libslic3r/TriangleMesh.cpp xs/src/libslic3r/Polygon.cpp xs/src/libslic3r/ExPolygon.cpp \
  xs/src/libslic3r/Point.cpp xs/src/libslic3r/Multipoint.cpp \
  xs/src/libslic3r/TransformationMatrix.cpp \
  xs/src/libslic3r/ClipperUtils.cpp xs/src/libslic3r/Log.cpp \
  xs/src/admesh/connect.c xs/src/admesh/normals.c xs/src/admesh/shared.c \
  xs/src/admesh/stlinit.c xs/src/admesh/stl_io.c xs/src/admesh/util.c \
  xs/src/clipper.cpp \
  xs/src/poly2tri/common/shapes.cc \
  xs/src/poly2tri/sweep/sweep.cc xs/src/poly2tri/sweep/sweep_context.cc xs/src/poly2tri/sweep/cdt.cc xs/src/poly2tri/sweep/advancing_front.cc \
  draco/src/draco/mesh/triangle_soup_mesh_builder.cc \
  draco/src/draco/compression/mesh/mesh_encoder.cc \
  draco/src/draco/compression/mesh/mesh_edgebreaker_encoder.cc \
  draco/src/draco/compression/mesh/mesh_edgebreaker_encoder_impl.cc \
  draco/src/draco/compression/point_cloud/point_cloud_encoder.cc \
  draco/src/draco/point_cloud/point_cloud.cc \
  draco/src/draco/core/encoder_buffer.cc \
  draco/src/draco/core/decoder_buffer.cc \
  draco/src/draco/attributes/geometry_attribute.cc \
  draco/src/draco/mesh/mesh.cc \
  draco/src/draco/core/options.cc \
  draco/src/draco/compression/bit_coders/rans_bit_encoder.cc \
  draco/src/draco/compression/bit_coders/rans_bit_decoder.cc \
  draco/src/draco/metadata/metadata_encoder.cc \
  draco/src/draco/metadata/metadata_decoder.cc \
  draco/src/draco/compression/attributes/sequential_attribute_encoders_controller.cc \
  draco/src/draco/mesh/corner_table.cc \
  draco/src/draco/core/divide.cc \
  draco/src/draco/compression/attributes/attributes_encoder.cc \
  draco/src/draco/attributes/point_attribute.cc \
  draco/src/draco/mesh/mesh_attribute_corner_table.cc \
  draco/src/draco/mesh/mesh_misc_functions.cc \
  draco/src/draco/compression/attributes/sequential_quantization_attribute_encoder.cc \
  draco/src/draco/compression/attributes/sequential_integer_attribute_encoder.cc \
  draco/src/draco/compression/attributes/sequential_attribute_encoder.cc \
  draco/src/draco/core/draco_types.cc \
  draco/src/draco/attributes/attribute_quantization_transform.cc \
  draco/src/draco/core/quantization_utils.cc \
  draco/src/draco/compression/entropy/shannon_entropy.cc \
  draco/src/draco/compression/attributes/prediction_schemes/prediction_scheme_encoder_factory.cc \
  draco/src/draco/core/bit_utils.cc \
  draco/src/draco/core/data_buffer.cc \
  draco/src/draco/compression/entropy/symbol_encoding.cc \
  draco/src/draco/compression/entropy/symbol_decoding.cc \
  draco/src/draco/attributes/attribute_transform.cc \
  draco/src/draco/compression/mesh/mesh_edgebreaker_decoder.cc \
  draco/src/draco/compression/point_cloud/point_cloud_decoder.cc \
  draco/src/draco/compression/mesh/mesh_decoder.cc \
  draco/src/draco/compression/mesh/mesh_edgebreaker_decoder_impl.cc \
  draco/src/draco/metadata/geometry_metadata.cc \
  draco/src/draco/metadata/metadata.cc \
  draco/src/draco/compression/attributes/sequential_attribute_decoders_controller.cc \
  draco/src/draco/compression/attributes/attributes_decoder.cc \
  draco/src/draco/compression/attributes/sequential_attribute_decoder.cc \
  draco/src/draco/compression/attributes/sequential_integer_attribute_decoder.cc \
  draco/src/draco/compression/attributes/sequential_quantization_attribute_decoder.cc \
  -Ixs/src/libslic3r -Ixs/src -Isrc/standalone -I/usr/local/Cellar/boost/1.72.0/include \
  -Idraco/src \
  --pre-js prefix.js --post-js postfix.js \
  -o bin/mc.js

#  util.cc FastNoise.cpp noise.cc cachedNoise.cc compose.cc march.cc tssl.cc light.cc heightfield.cc flod.cc noiser.cc cull.cc objectize.cc \
#  --pre-js prefix.js --post-js postfix.js \
#  -o bin/objectize.js
