#include "cut.h"

using namespace Slic3r;

void cut(
  float *positions,
  unsigned int numPositions,
  unsigned int *faces,
  unsigned int numFaces,
  float *position,
  float *quaternion,
  float *scale,
  float *outPositions,
  unsigned int *numOutPositions,
  unsigned int *outFaces,
  unsigned int *numOutFaces
) {
  TriangleMesh mesh(positions, faces, numFaces);
  TransformationMatrix matrix = TransformationMatrix::multiply(
    TransformationMatrix::mat_translation(position[0], position[1], position[2]),
    TransformationMatrix::multiply(
      TransformationMatrix::mat_rotation(quaternion[0], quaternion[1], quaternion[2], quaternion[3]),
      TransformationMatrix::mat_scale(scale[0], scale[1], scale[2])
    ));
  TransformationMatrix matrixInverse = matrix.inverse();
  mesh.transform(matrixInverse);

  TriangleMesh upper;
  TriangleMesh lower;
  mesh.cut(Axis::Y, 0, &upper, &lower);
  upper.repair();
  lower.repair();

  {
    const std::vector<Pointf3> &upperPositions = upper.vertices();
    numOutPositions[0] = 0;
    for (size_t i = 0; i < upperPositions.size(); i++) {
      const Pointf3 &p = matrix.transform(upperPositions[i]);
      outPositions[numOutPositions[0]++] = p.x;
      outPositions[numOutPositions[0]++] = p.y;
      outPositions[numOutPositions[0]++] = p.z;
    }

    const std::vector<Point3> &upperIndices = upper.facets();
    memcpy(outFaces, upperIndices.data(), upperIndices.size()*3*sizeof(upperIndices[0]));
    numOutFaces[0] = upperIndices.size()*3;
  }
  {
    const std::vector<Pointf3> &upperPositions = lower.vertices();
    numOutPositions[1] = 0;
    for (size_t i = 0; i < upperPositions.size(); i++) {
      const Pointf3 &p = matrix.transform(upperPositions[i]);
      outPositions[numOutPositions[0] + numOutPositions[1]++] = p.x;
      outPositions[numOutPositions[0] + numOutPositions[1]++] = p.y;
      outPositions[numOutPositions[0] + numOutPositions[1]++] = p.z;
    }

    const std::vector<Point3> &upperIndices = lower.facets();
    memcpy(outFaces + numOutFaces[0], upperIndices.data(), upperIndices.size()*3*sizeof(upperIndices[1]));
    numOutFaces[1] = upperIndices.size()*3;
  }

  /* csgjs_plane plane;
  plane.normal = csgjs_vector(0, 1, 0);
  plane.w = 0.5;

  csgjs_polygon polygon;

  std::vector<csgjs_polygon> coplanarFront;
  std::vector<csgjs_polygon> coplanarBack;
  std::vector<csgjs_polygon> front;
  std::vector<csgjs_polygon> back;
  plane.splitPolygon(polygon, coplanarFront, coplanarBack, front, back); */
}

/* std::vector<float> getMeshVector(TriangleMesh &mesh, const std::vector<float> &v) {
  const std::vector<int> &originalFacets = mesh.originalFacets();
  std::vector<float> result(originalFacets.size()*9);
  int index = 0;
  for (size_t i = 0; i < originalFacets.size(); i++) {
    int originalIndex = originalFacets[i];

    if (originalIndex != -1) {
      memcpy(&result[index], &v[originalIndex*9], 9*sizeof(float));
    }
    index += 9;
  }
  return std::move(result);
} */

void chunk(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  float *uvs,
  unsigned int numUvs,
  float *mins,
  float *maxs,
  float *scale,
  float **outPositions,
  unsigned int *numOutPositions,
  float **outNormals,
  unsigned int *numOutNormals,
  float **outColors,
  unsigned int *numOutColors,
  float **outUvs,
  unsigned int *numOutUvs
) {
  unsigned int numFaces = numPositions/3;
  std::vector<unsigned int> faces(numFaces);
  for (size_t i = 0; i < numFaces; i++) {
    faces[i] = i;
  }
  TriangleMesh hMesh(positions, faces.data(), numFaces);

  int meshIndex = 0;
  for (float x = mins[0]; x < maxs[0]; x += scale[0]) {
    float ax = x + scale[0];

    TriangleMesh left;
    TriangleMesh right;
    hMesh.cut(Axis::X, ax, &right, &left);
    left.repair();
    right.repair();

    TriangleMesh &vMesh = left;

    for (float z = mins[2]; z < maxs[2]; z += scale[2]) {
      float az = z + scale[2];

      TriangleMesh top;
      TriangleMesh bottom;
      // mesh = TriangleMesh(positions, faces.data(), numFaces);
      vMesh.cut(Axis::Z, az, &top, &bottom);
      top.repair();
      bottom.repair();

      /* std::vector<float> topNormals = getMeshVector(top, vNormalsVector);
      std::vector<float> topColors = getMeshVector(top, vColorsVector);
      std::vector<float> bottomNormals = getMeshVector(bottom, vNormalsVector);
      std::vector<float> bottomColors = getMeshVector(bottom, vColorsVector); */

      float *outP = outPositions[meshIndex];
      unsigned int *numOutP = &numOutPositions[meshIndex];
      float *outN = outNormals[meshIndex];
      unsigned int *numOutN = &numOutNormals[meshIndex];
      float *outC = outColors[meshIndex];
      unsigned int *numOutC = &numOutColors[meshIndex];
      float *outU = outUvs[meshIndex];
      unsigned int *numOutU = &numOutUvs[meshIndex];
      meshIndex++;

      numOutP[0] = 0;
      numOutN[0] = 0;
      numOutC[0] = 0;
      numOutU[0] = 0;
      
      /* TransformationMatrix matrix = TransformationMatrix::multiply(
        TransformationMatrix::mat_translation(position[0], position[1], position[2]),
        TransformationMatrix::multiply(
          TransformationMatrix::mat_rotation(quaternion[0], quaternion[1], quaternion[2], quaternion[3]),
          TransformationMatrix::mat_scale(scale[0], scale[1], scale[2])
        ));
      TransformationMatrix matrixInverse = matrix.inverse();
      mesh.transform(matrixInverse); */

      const std::vector<Pointf3> &bottomPositions = bottom.vertices();

      /* for (size_t i = 0; i < topPositions.size(); i++) {
        const Pointf3 &topPosition = topPositions[i];
        outP[numOutP[0]++] = topPosition.x;
        outP[numOutP[0]++] = topPosition.y;
        outP[numOutP[0]++] = topPosition.z;
      } */

      const std::vector<Point3> &bottomIndices = bottom.facets();
      const std::vector<int> &bottomOriginalIndices = bottom.originalFacets();
      for (size_t i = 0; i < bottomIndices.size(); i++) {
        const Slic3r::Point3 &facet = bottomIndices[i];
        int originalIndex = bottomOriginalIndices[i];

        // positions
        outP[numOutP[0]++] = bottomPositions[facet.x].x;
        outP[numOutP[0]++] = bottomPositions[facet.x].y;
        outP[numOutP[0]++] = bottomPositions[facet.x].z;

        outP[numOutP[0]++] = bottomPositions[facet.y].x;
        outP[numOutP[0]++] = bottomPositions[facet.y].y;
        outP[numOutP[0]++] = bottomPositions[facet.y].z;

        outP[numOutP[0]++] = bottomPositions[facet.z].x;
        outP[numOutP[0]++] = bottomPositions[facet.z].y;
        outP[numOutP[0]++] = bottomPositions[facet.z].z;

        if (originalIndex != -1) {
          // normals
          outN[numOutN[0]++] = normals[originalIndex*9];
          outN[numOutN[0]++] = normals[originalIndex*9+1];
          outN[numOutN[0]++] = normals[originalIndex*9+2];

          outN[numOutN[0]++] = normals[originalIndex*9+3];
          outN[numOutN[0]++] = normals[originalIndex*9+4];
          outN[numOutN[0]++] = normals[originalIndex*9+5];

          outN[numOutN[0]++] = normals[originalIndex*9+6];
          outN[numOutN[0]++] = normals[originalIndex*9+7];
          outN[numOutN[0]++] = normals[originalIndex*9+8];

          // colors
          outC[numOutC[0]++] = colors[originalIndex*9];
          outC[numOutC[0]++] = colors[originalIndex*9+1];
          outC[numOutC[0]++] = colors[originalIndex*9+2];

          outC[numOutC[0]++] = colors[originalIndex*9+3];
          outC[numOutC[0]++] = colors[originalIndex*9+4];
          outC[numOutC[0]++] = colors[originalIndex*9+5];

          outC[numOutC[0]++] = colors[originalIndex*9+6];
          outC[numOutC[0]++] = colors[originalIndex*9+7];
          outC[numOutC[0]++] = colors[originalIndex*9+8];

          // uvs
          outU[numOutU[0]++] = uvs[originalIndex*6];
          outU[numOutU[0]++] = uvs[originalIndex*6+1];

          outU[numOutU[0]++] = uvs[originalIndex*6+2];
          outU[numOutU[0]++] = uvs[originalIndex*6+3];

          outU[numOutU[0]++] = uvs[originalIndex*6+4];
          outU[numOutU[0]++] = uvs[originalIndex*6+5];
        } else {
          // normals
          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 0;

          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 0;

          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 0;

          // colors
          outC[numOutC[0]++] = 0;
          outC[numOutC[0]++] = 0;
          outC[numOutC[0]++] = 0;

          outC[numOutC[0]++] = 0;
          outC[numOutC[0]++] = 0;
          outC[numOutC[0]++] = 0;

          outC[numOutC[0]++] = 0;
          outC[numOutC[0]++] = 0;
          outC[numOutC[0]++] = 0;

          // uvs
          outU[numOutU[0]++] = 0;
          outU[numOutU[0]++] = 0;

          outU[numOutU[0]++] = 0;
          outU[numOutU[0]++] = 0;

          outU[numOutU[0]++] = 0;
          outU[numOutU[0]++] = 0;
        }
      }

      vMesh = std::move(top);
      // vNormalsVector = std::move(topNormals);
      // vColorsVector = std::move(topColors);
    }

    hMesh = std::move(right);
    // hNormalsVector = std::move(rightNormals);
    // hColorsVector = std::move(rightColors);
  }
}

struct Vertex {
  float position[3];
  float normal[3];
  float color[3];
  float uv[2];
};

void decimate(
  float *positions,
  unsigned int &numPositions,
  float *normals,
  unsigned int &numNormals,
  float *colors,
  unsigned int &numColors,
  float *uvs,
  unsigned int &numUvs,
  float factor,
  float target_error,
  unsigned int *faces,
  unsigned int &numFaces
) {
  size_t total_indices = numPositions/3;
  // std::cerr << "get index count 1 " << numPositions << " " << index_count << std::endl;

  std::vector<unsigned int> remap(total_indices);
  size_t total_vertices;
  {
    std::vector<Vertex> vertices(total_indices);
    for (size_t i = 0; i < total_indices; i++) {
      vertices[i] = Vertex{
        {
          positions[i*3],
          positions[i*3+1],
          positions[i*3+2],
        },
        {
          normals[i*3],
          normals[i*3+1],
          normals[i*3+2],
        },
        {
          colors[i*3],
          colors[i*3+1],
          colors[i*3+2],
        },
        {
          uvs[i*2],
          uvs[i*2+1]
        },
      };
    }
    total_vertices = meshopt_generateVertexRemap(remap.data(), NULL, total_indices, vertices.data(), total_indices, sizeof(Vertex));
  }
  // total_vertices = meshopt_generateVertexRemap(remap.data(), NULL, total_indices, positions, total_indices, 3*sizeof(float));

  std::vector<unsigned int> facesTmp(total_indices);
  meshopt_remapIndexBuffer(facesTmp.data(), NULL, total_indices, &remap[0]);

  meshopt_remapVertexBuffer(positions, positions, total_indices, sizeof(float) * 3, &remap[0]);
  numPositions = total_vertices * 3;

  meshopt_remapVertexBuffer(normals, normals, total_indices, sizeof(float) * 3, &remap[0]);
  numNormals = total_vertices * 3;

  meshopt_remapVertexBuffer(colors, colors, total_indices, sizeof(float) * 3, &remap[0]);
  numColors = total_vertices * 3;

  meshopt_remapVertexBuffer(uvs, uvs, total_indices, sizeof(float) * 2, &remap[0]);
  numUvs = total_vertices * 2;

  size_t target_index_count = size_t(facesTmp.size() * factor);
  numFaces = meshopt_simplify(faces, facesTmp.data(), facesTmp.size(), positions, numPositions, 3*sizeof(float), target_index_count, target_error);
  // numFaces = meshopt_simplifySloppy(faces, facesTmp.data(), facesTmp.size(), positions, numPositions, 3*sizeof(float), target_index_count);

  const unsigned int oldNumFaces = numFaces;
  const unsigned int maxIndex = numPositions/3;
  unsigned int *faceWrite = faces;
  for (size_t i = 0; i < oldNumFaces; i += 3) {
    if (faces[i] < maxIndex && faces[i+1] < maxIndex && faces[i+2] < maxIndex) {
      faceWrite[0] = faces[i];
      faceWrite[1] = faces[i+1];
      faceWrite[2] = faces[i+2];
      faceWrite += 3;
    } else {
      numFaces -= 3;
    }
  }
}