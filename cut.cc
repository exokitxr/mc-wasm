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

std::vector<float> getMeshVector(TriangleMesh &mesh, const std::vector<float> &v) {
  const std::vector<int> &originalFacets = mesh.originalFacets();
  std::vector<float> result(originalFacets.size()*9);
  int index = 0;
  for (size_t i = 0; i < originalFacets.size(); i++) {
    int originalIndex = originalFacets[i];

    if (originalIndex != -1) {
      memcpy(&result[index], &v[originalIndex*9], 9*sizeof(float));
      /* result[index++] = v[originalIndex*9];
      result[index++] = v[originalIndex*9+1];
      result[index++] = v[originalIndex*9+2];

      result[index++] = v[originalIndex*9+3];
      result[index++] = v[originalIndex*9+4];
      result[index++] = v[originalIndex*9+5];

      result[index++] = v[originalIndex*9+6];
      result[index++] = v[originalIndex*9+7];
      result[index++] = v[originalIndex*9+8]; */
    } /* else {
      result[index++] = 0;
      result[index++] = 0;
      result[index++] = 0;

      result[index++] = 0;
      result[index++] = 0;
      result[index++] = 0;

      result[index++] = 0;
      result[index++] = 0;
      result[index++] = 0;
    } */
    index += 9;
  }
  return std::move(result);
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
  TriangleMesh hMesh(positions, faces.data(), numFaces);

  int meshIndex = 0;

  std::cerr << "cut 1 " << numNormals << " " << numColors << std::endl;

  std::vector<float> hNormalsVector(numNormals);
  std::cerr << "cut 1.1 " << numNormals << " " << numColors << std::endl;
  memcpy(hNormalsVector.data(), normals, numNormals*sizeof(float));
  std::cerr << "cut 1.2 " << numNormals << " " << numColors << std::endl;
  std::vector<float> hColorsVector(numColors);
  std::cerr << "cut 1.3 " << numNormals << " " << numColors << std::endl;
  memcpy(hColorsVector.data(), colors, numColors*sizeof(float));
  std::cerr << "cut 1.4 " << numNormals << " " << numColors << std::endl;

  std::cerr << "cut 2" << std::endl;

  for (float x = mins[0]; x < maxs[0]; x += scale[0]) {
    float ax = x + scale[0];

    std::cerr << "cut 3 " << ax << std::endl;

    TriangleMesh left;
    TriangleMesh right;
    // mesh = TriangleMesh(positions, faces.data(), numFaces);
    hMesh.cut(Axis::X, ax, &right, &left);
    left.repair();
    right.repair();

    std::vector<float> leftNormals = getMeshVector(left, hNormalsVector);
    std::vector<float> leftColors = getMeshVector(left, hColorsVector);
    std::vector<float> rightNormals = getMeshVector(right, hNormalsVector);
    std::vector<float> rightColors = getMeshVector(right, hColorsVector);

    TriangleMesh &vMesh = left;
    std::vector<float> &vNormalsVector = leftNormals;
    std::vector<float> &vColorsVector = leftColors;

    for (float z = mins[2]; z < maxs[2]; z += scale[2]) {
      float az = z + scale[2];

      std::cerr << "cut 4 " << az << std::endl;

      TriangleMesh top;
      TriangleMesh bottom;
      // mesh = TriangleMesh(positions, faces.data(), numFaces);
      vMesh.cut(Axis::Z, az, &top, &bottom);
      top.repair();
      bottom.repair();

      std::cerr << "cut 5" << std::endl;

      std::vector<float> topNormals = getMeshVector(top, vNormalsVector);
      std::vector<float> topColors = getMeshVector(top, vColorsVector);
      std::vector<float> bottomNormals = getMeshVector(bottom, vNormalsVector);
      std::vector<float> bottomColors = getMeshVector(bottom, vColorsVector);

      std::cerr << "cut 6" << std::endl;

      float *outP = outPositions[meshIndex];
      unsigned int *numOutP = &numOutPositions[meshIndex];
      float *outN = outNormals[meshIndex];
      unsigned int *numOutN = &numOutNormals[meshIndex];
      float *outC = outColors[meshIndex];
      unsigned int *numOutC = &numOutColors[meshIndex];
      meshIndex++;

      numOutP[0] = 0;
      numOutN[0] = 0;
      numOutC[0] = 0;

      std::cerr << "cut 7" << std::endl;
      
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
        }
      }

      std::cerr << "cut 8" << std::endl;

      vMesh = std::move(top);
      vNormalsVector = std::move(topNormals);
      vColorsVector = std::move(topColors);

      std::cerr << "cut 9" << std::endl;
    }

    hMesh = std::move(right);
    hNormalsVector = std::move(rightNormals);
    hColorsVector = std::move(rightColors);
  }
  std::cerr << "cut 11" << std::endl;
}