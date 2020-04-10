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

void chunk(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  float *mins,
  float *maxs,
  float *scale,
  float **outPositions,
  unsigned int *numOutPositions,
  float **outNormals,
  unsigned int *numOutNormals,
  float **outColors,
  unsigned int *numOutColors
) {
  unsigned int numFaces = numPositions/3;
  std::vector<unsigned int> faces(numFaces);
  for (size_t i = 0; i < numFaces; i++) {
    faces[i] = i;
  }
  TriangleMesh mesh(positions, faces.data(), numFaces);

  int meshIndex = 0;
  for (float x = mins[0]; x < maxs[0]; x++) {
    for (float z = mins[2]; z < maxs[2]; z++) {
      // float *outP = outPositions[meshIndex];
      unsigned int *numOutP = &numOutPositions[meshIndex];
      // float *outN = outNormals[meshIndex];
      unsigned int *numOutN = &numOutNormals[meshIndex];
      // float *outC = outColors[meshIndex];
      unsigned int *numOutC = &numOutColors[meshIndex];
      meshIndex++;

      numOutP[0] = 0;
      numOutN[0] = 0;
      numOutC[0] = 0;
    }
  }
  meshIndex = 0;

  for (float x = mins[0]; x < maxs[0]; x++) {
    float ax = x * scale[0];
    ax *= scale[0];

    float *outP = outPositions[meshIndex];
    unsigned int *numOutP = &numOutPositions[meshIndex];
    float *outN = outNormals[meshIndex];
    unsigned int *numOutN = &numOutNormals[meshIndex];
    float *outC = outColors[meshIndex];
    unsigned int *numOutC = &numOutColors[meshIndex];
    meshIndex++;

    std::cerr << "cut 1 " << ax << std::endl;
    
    /* TransformationMatrix matrix = TransformationMatrix::multiply(
      TransformationMatrix::mat_translation(position[0], position[1], position[2]),
      TransformationMatrix::multiply(
        TransformationMatrix::mat_rotation(quaternion[0], quaternion[1], quaternion[2], quaternion[3]),
        TransformationMatrix::mat_scale(scale[0], scale[1], scale[2])
      ));
    TransformationMatrix matrixInverse = matrix.inverse();
    mesh.transform(matrixInverse); */

    TriangleMesh left;
    TriangleMesh right;
    // mesh = TriangleMesh(positions, faces.data(), numFaces);
    mesh.cut(Axis::X, ax, &right, &left);
    left.repair();
    right.repair();

    {
      const std::vector<Pointf3> &topPositions = left.vertices();
      numOutP[0] = 0;
      numOutN[0] = 0;
      numOutC[0] = 0;

      /* for (size_t i = 0; i < topPositions.size(); i++) {
        const Pointf3 &topPosition = topPositions[i];
        outP[numOutP[0]++] = topPosition.x;
        outP[numOutP[0]++] = topPosition.y;
        outP[numOutP[0]++] = topPosition.z;
      } */

      const std::vector<Point3> &topIndices = left.facets();
      const std::vector<int> &topOriginalIndices = left.originalFacets();
      for (size_t i = 0; i < topIndices.size(); i++) {
        const Slic3r::Point3 &facet = topIndices[i];
        int originalIndex = topOriginalIndices[i];

        // positions
        outP[numOutP[0]++] = topPositions[facet.x].x;
        outP[numOutP[0]++] = topPositions[facet.x].y;
        outP[numOutP[0]++] = topPositions[facet.x].z;

        outP[numOutP[0]++] = topPositions[facet.y].x;
        outP[numOutP[0]++] = topPositions[facet.y].y;
        outP[numOutP[0]++] = topPositions[facet.y].z;

        outP[numOutP[0]++] = topPositions[facet.z].x;
        outP[numOutP[0]++] = topPositions[facet.z].y;
        outP[numOutP[0]++] = topPositions[facet.z].z;

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
        } else {
          // normals
          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 1;
          outN[numOutN[0]++] = 0;

          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 1;
          outN[numOutN[0]++] = 0;

          outN[numOutN[0]++] = 0;
          outN[numOutN[0]++] = 1;
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
        }
      }
    }
  }
  std::cerr << "cut 11" << std::endl;
}