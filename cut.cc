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
  unsigned int **numOutPositions,
  float **outNormals,
  unsigned int **numOutNormals,
  float **outColors,
  unsigned int **numOutColors
) {
  unsigned int numFaces = numPositions/3;
  std::vector<unsigned int> faces(numFaces);
  for (size_t i = 0; i < numFaces; i++) {
    faces[i] = i;
  }
  TriangleMesh mesh(positions, faces.data(), numFaces);

  for (float x = mins[0]; x < maxs[0]; x++) {
    float ax = x * scale[0];
    ax *= scale[0];
    
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
    mesh.cut(Axis::X, ax, &right, &left);
    left.repair();
    right.repair();

    int meshIndex = 0;
    for (float z = mins[2]; z < maxs[2]; z++) {
      float az = z * scale[2];
      az *= scale[2];

      TriangleMesh top;
      TriangleMesh bottom;
      right.cut(Axis::Z, az, &top, &bottom);
      top.repair();
      bottom.repair();

      float *outP = outPositions[meshIndex];
      unsigned int *numOutP = numOutPositions[meshIndex];
      float *outN = outNormals[meshIndex];
      unsigned int *numOutN = numOutNormals[meshIndex];
      float *outC = outColors[meshIndex];
      unsigned int *numOutC = numOutColors[meshIndex];
      meshIndex++;

      {
        const std::vector<Pointf3> &topPositions = top.vertices();
        numOutP[0] = 0;
        for (size_t i = 0; i < topPositions.size(); i++) {
          // const Pointf3 &p = matrix.transform(topPositions[i]);
          const Pointf3 &p = topPositions[i];
          outP[numOutP[0]++] = p.x;
          outP[numOutP[0]++] = p.y;
          outP[numOutP[0]++] = p.z;
        }

        const std::vector<Point3> &topIndices = top.facets();
        for (size_t i = 0; i < topIndices.size(); i++) {
          const Slic3r::Point3 &facet = topIndices[i];

          // normals
          outN[numOutN[0]++] = normals[facet.x*3];
          outN[numOutN[0]++] = normals[facet.x*3+1];
          outN[numOutN[0]++] = normals[facet.x*3+2];

          outN[numOutN[0]++] = normals[facet.y*3];
          outN[numOutN[0]++] = normals[facet.y*3+1];
          outN[numOutN[0]++] = normals[facet.y*3+2];

          outN[numOutN[0]++] = normals[facet.z*3];
          outN[numOutN[0]++] = normals[facet.z*3+1];
          outN[numOutN[0]++] = normals[facet.z*3+2];

          // colors
          outC[numOutC[0]++] = colors[facet.x*3];
          outC[numOutC[0]++] = colors[facet.x*3+1];
          outC[numOutC[0]++] = colors[facet.x*3+2];

          outC[numOutC[0]++] = colors[facet.y*3];
          outC[numOutC[0]++] = colors[facet.y*3+1];
          outC[numOutC[0]++] = colors[facet.y*3+2];

          outC[numOutC[0]++] = colors[facet.z*3];
          outC[numOutC[0]++] = colors[facet.z*3+1];
          outC[numOutC[0]++] = colors[facet.z*3+2];
        }
      }
    }

    mesh = right;
  }
}