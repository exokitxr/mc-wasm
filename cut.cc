#include "cut.h"
#include "Simplify.h"
#include <unordered_set>

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
  unsigned int *ids,
  unsigned int numIds,
  unsigned int *faces,
  unsigned int numFaces,
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
  unsigned int *numOutUvs,
  unsigned int **outIds,
  unsigned int *numOutIds,
  unsigned int **outFaces,
  unsigned int *numOutFaces
) {
  TriangleMesh hMesh(positions, faces, numFaces);

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
      vMesh.cut(Axis::Z, az, &top, &bottom);
      top.repair();
      bottom.repair();

      float *outP = outPositions[meshIndex];
      unsigned int *numOutP = &numOutPositions[meshIndex];
      float *outN = outNormals[meshIndex];
      unsigned int *numOutN = &numOutNormals[meshIndex];
      float *outC = outColors[meshIndex];
      unsigned int *numOutC = &numOutColors[meshIndex];
      float *outU = outUvs[meshIndex];
      unsigned int *numOutU = &numOutUvs[meshIndex];
      unsigned int *outX = outIds[meshIndex];
      unsigned int *numOutX = &numOutIds[meshIndex];
      unsigned int *outI = outFaces[meshIndex];
      unsigned int *numOutI = &numOutFaces[meshIndex];
      meshIndex++;

      numOutP[0] = 0;
      numOutN[0] = 0;
      numOutC[0] = 0;
      numOutU[0] = 0;
      numOutX[0] = 0;
      numOutI[0] = 0;

      const std::vector<Pointf3> &bottomPositions = bottom.vertices();

      const std::vector<Point3> &bottomIndices = bottom.facets();
      const std::vector<int> &bottomOriginalIndices = bottom.originalFacets();
      for (size_t i = 0; i < bottomIndices.size(); i++) {
        const Slic3r::Point3 &facet = bottomIndices[i];
        int originalFacet = bottomOriginalIndices[i];

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

        if (originalFacet != -1) {
          unsigned int originalIndex[3] = {
            faces[originalFacet*3],
            faces[originalFacet*3+1],
            faces[originalFacet*3+2],
          };

          // normals
          outN[numOutN[0]++] = normals[originalIndex[0]*3];
          outN[numOutN[0]++] = normals[originalIndex[0]*3+1];
          outN[numOutN[0]++] = normals[originalIndex[0]*3+2];

          outN[numOutN[0]++] = normals[originalIndex[1]*3];
          outN[numOutN[0]++] = normals[originalIndex[1]*3+1];
          outN[numOutN[0]++] = normals[originalIndex[1]*3+2];

          outN[numOutN[0]++] = normals[originalIndex[2]*3];
          outN[numOutN[0]++] = normals[originalIndex[2]*3+1];
          outN[numOutN[0]++] = normals[originalIndex[2]*3+2];

          // colors
          outC[numOutC[0]++] = colors[originalIndex[0]*3];
          outC[numOutC[0]++] = colors[originalIndex[0]*3+1];
          outC[numOutC[0]++] = colors[originalIndex[0]*3+2];

          outC[numOutC[0]++] = colors[originalIndex[1]*3];
          outC[numOutC[0]++] = colors[originalIndex[1]*3+1];
          outC[numOutC[0]++] = colors[originalIndex[1]*3+2];

          outC[numOutC[0]++] = colors[originalIndex[2]*3];
          outC[numOutC[0]++] = colors[originalIndex[2]*3+1];
          outC[numOutC[0]++] = colors[originalIndex[2]*3+2];

          // uvs
          outU[numOutU[0]++] = uvs[originalIndex[0]*2];
          outU[numOutU[0]++] = uvs[originalIndex[0]*2+1];

          outU[numOutU[0]++] = uvs[originalIndex[1]*2];
          outU[numOutU[0]++] = uvs[originalIndex[1]*2+1];

          outU[numOutU[0]++] = uvs[originalIndex[2]*2];
          outU[numOutU[0]++] = uvs[originalIndex[2]*2+1];

          // ids
          outX[numOutX[0]++] = ids[originalIndex[0]];

          outX[numOutX[0]++] = ids[originalIndex[1]];

          outX[numOutX[0]++] = ids[originalIndex[2]];
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

          // ids
          outX[numOutX[0]++] = 0;

          outX[numOutX[0]++] = 0;

          outX[numOutX[0]++] = 0;
        }
      }

      vMesh = std::move(top);
    }

    hMesh = std::move(right);
  }
}

class HashVec {
public:
  int x;
  int y;
  int z;
  unsigned int index;
};
bool operator==(const HashVec &a, const HashVec &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}
class Hash {
public:
   size_t operator() (const HashVec &a) const {
     return *(size_t *)(&a.x) ^ *(size_t *)(&a.y) ^ *(size_t *)(&a.z);
   }
};

void chunkOne(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  float *uvs,
  unsigned int numUvs,
  unsigned int *ids,
  unsigned int numIds,
  unsigned int *faces,
  unsigned int numFaces,
  float *mins,
  float *maxs,
  float *outP,
  unsigned int *numOutP,
  float *outN,
  unsigned int *numOutN,
  float *outC,
  unsigned int *numOutC,
  float *outU,
  unsigned int *numOutU,
  unsigned int *outX,
  unsigned int *numOutX,
  unsigned int *outI,
  unsigned int *numOutI
) {
  TriangleMesh mesh(positions, faces, numFaces);

  // {
    // TriangleMesh left1;
    TriangleMesh right1;
    mesh.cut(Axis::X, mins[0], &right1, nullptr);
    mesh = TriangleMesh();
    // left1.repair();
    // right1.repair();
    // mesh = right;
    // mesh.repair();
  // }
  // {
    TriangleMesh left2;
    // TriangleMesh right2;
    right1.cut(Axis::X, maxs[0], nullptr, &left2);
    right1 = TriangleMesh();
    // left2.repair();
    // right2.repair();
    // mesh = left;
    // mesh.repair();
  // }
  // {
    TriangleMesh top1;
    // TriangleMesh bottom1;
    left2.cut(Axis::Z, mins[2], &top1, nullptr);
    left2 = TriangleMesh();
    // top1.repair();
    // bottom1.repair();
    // mesh = top;
    // mesh.repair();
  // }
  // {
   //  TriangleMesh top2;
    TriangleMesh bottom2;
    top1.cut(Axis::Z, maxs[2], nullptr, &bottom2);
    top1 = TriangleMesh();
    // top2.repair();
    bottom2.repair();
    // mesh = bottom;
    // mesh.repair();
  // }

  numOutP[0] = 0;
  numOutN[0] = 0;
  numOutC[0] = 0;
  numOutU[0] = 0;
  numOutX[0] = 0;
  numOutI[0] = 0;

  const std::vector<Pointf3> &bottomPositions = bottom2.vertices();
  const std::vector<Point3> &bottomIndices = bottom2.facets();
  const std::vector<int> &bottomOriginalIndices = bottom2.originalFacets();
  for (size_t i = 0; i < bottomIndices.size(); i++) {
    const Slic3r::Point3 &facet = bottomIndices[i];
    int originalFacet = bottomOriginalIndices[i];

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

    if (originalFacet != -1) {
      unsigned int originalIndex[3] = {
        faces[originalFacet*3],
        faces[originalFacet*3+1],
        faces[originalFacet*3+2],
      };

      // normals
      outN[numOutN[0]++] = normals[originalIndex[0]*3];
      outN[numOutN[0]++] = normals[originalIndex[0]*3+1];
      outN[numOutN[0]++] = normals[originalIndex[0]*3+2];

      outN[numOutN[0]++] = normals[originalIndex[1]*3];
      outN[numOutN[0]++] = normals[originalIndex[1]*3+1];
      outN[numOutN[0]++] = normals[originalIndex[1]*3+2];

      outN[numOutN[0]++] = normals[originalIndex[2]*3];
      outN[numOutN[0]++] = normals[originalIndex[2]*3+1];
      outN[numOutN[0]++] = normals[originalIndex[2]*3+2];

      // colors
      outC[numOutC[0]++] = colors[originalIndex[0]*3];
      outC[numOutC[0]++] = colors[originalIndex[0]*3+1];
      outC[numOutC[0]++] = colors[originalIndex[0]*3+2];

      outC[numOutC[0]++] = colors[originalIndex[1]*3];
      outC[numOutC[0]++] = colors[originalIndex[1]*3+1];
      outC[numOutC[0]++] = colors[originalIndex[1]*3+2];

      outC[numOutC[0]++] = colors[originalIndex[2]*3];
      outC[numOutC[0]++] = colors[originalIndex[2]*3+1];
      outC[numOutC[0]++] = colors[originalIndex[2]*3+2];

      // uvs
      outU[numOutU[0]++] = uvs[originalIndex[0]*2];
      outU[numOutU[0]++] = uvs[originalIndex[0]*2+1];

      outU[numOutU[0]++] = uvs[originalIndex[1]*2];
      outU[numOutU[0]++] = uvs[originalIndex[1]*2+1];

      outU[numOutU[0]++] = uvs[originalIndex[2]*2];
      outU[numOutU[0]++] = uvs[originalIndex[2]*2+1];

      // ids
      outX[numOutX[0]++] = ids[originalIndex[0]];

      outX[numOutX[0]++] = ids[originalIndex[1]];

      outX[numOutX[0]++] = ids[originalIndex[2]];
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

      // ids
      outX[numOutX[0]++] = 0;

      outX[numOutX[0]++] = 0;

      outX[numOutX[0]++] = 0;
    }
  }
}

struct Vertex {
  float position[3];
  float normals[3];
};

void subVectors(float *c, const float *a, const float *b) {
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}
void crossVectors(float *c, const float *a, const float *b) {
  const float ax = a[0], ay = a[1], az = a[2];
  const float bx = b[0], by = b[1], bz = b[2];

  c[0] = ay * bz - az * by;
  c[1] = az * bx - ax * bz;
  c[2] = ax * by - ay * bx;
}

void decimate(
  float *positions,
  unsigned int &numPositions,
  float *normals,
  unsigned int &numNormals,
  float *colors,
  unsigned int &numColors,
  float *uvs,
  unsigned int &numUvs,
  unsigned int *ids,
  unsigned int &numIds,
  float minTris,
  float quantization,
  float target_error,
  float aggressiveness,
  float base,
  int iterationOffset,
  unsigned int *faces,
  unsigned int &numFaces
) {
  std::cerr << "decimate 0" << std::endl;
  if ((unsigned int)minTris >= numPositions/9) {
    return;
  }

  size_t total_indices;
  for (;;) {
    std::cerr << "decimate 1 " << quantization << " " << numPositions << std::endl;
    /* for (size_t i = 0; i < numPositions; i++) {
      positions[i] = std::round(positions[i]/quantization)*quantization;
    } */

    {
      std::unordered_set<HashVec, Hash> points;
      int numInserted = 0;
      int numReferenced = 0;
      {
        // std::vector<unsigned int> remap(numPositions/3);
        // std::vector<bool> keepFaces(numPositions/3);
        unsigned int i = 0;
        unsigned int index = 0;
        for (; i < numPositions; i += 3, index++) {
          HashVec v{
            (int)(positions[i]/quantization),
            (int)(positions[i+1]/quantization),
            (int)(positions[i+2]/quantization),
            index,
          };
          auto iterPair = points.insert(v);
          auto &iter = iterPair.first;
          auto &inserted = iterPair.second;
          if (inserted) {
            faces[index] = index;
            numInserted++;
          } else {
            /* unsigned int matchingIndex = iter->index;
            positions[i] = positions[matchingIndex*3];
            positions[i+1] = positions[matchingIndex*3+1];
            positions[i+2] = positions[matchingIndex*3+2]; */
            faces[index] = iter->index;
            numReferenced++;
          }
        }
      }
      std::cerr << "decimate 2.1 " << quantization << " " << numPositions << " " << points.size() << " " << numInserted << " " << numReferenced << std::endl;
      /* for (unsigned int i = 0; i < numPositions/3; i++) {
        std::cerr << " " << faces[i] << " " << positions[faces[i]*3] << " " << positions[faces[i]*3+1] << " " << positions[faces[i]*3+2] << std::endl;
      } */
      const unsigned int numTris = numPositions/9;
      std::vector<unsigned char> deadTris(numTris);
      unsigned int numDeadTris = 0;
      {
        unsigned int i = 0;
        unsigned int index = 0;
        for (; index < numTris; i += 3, index++) {
          if (faces[i] == faces[i+1] && faces[i] == faces[i+2]) {
            deadTris[index] = true;
            numDeadTris++;
          }
        }
      }
      std::cerr << "decimate 2.2 " << quantization << " " << numPositions << " " << points.size() << " " << numDeadTris << std::endl;
      if ((numTris - numDeadTris) < ((size_t)minTris)) {
        numPositions = 0;
        numNormals = 0;
        numColors = 0;
        numUvs = 0;
        numIds = 0;

        for (size_t originalIndex = 0; originalIndex < numTris; originalIndex++) {
          if (!deadTris[originalIndex]) {
            unsigned int newIndexA = faces[originalIndex*3];
            unsigned int newIndexB = faces[originalIndex*3+1];
            unsigned int newIndexC = faces[originalIndex*3+2];

            positions[numPositions++] = positions[newIndexA*3];
            positions[numPositions++] = positions[newIndexA*3+1];
            positions[numPositions++] = positions[newIndexA*3+2];

            positions[numPositions++] = positions[newIndexB*3];
            positions[numPositions++] = positions[newIndexB*3+1];
            positions[numPositions++] = positions[newIndexB*3+2];

            positions[numPositions++] = positions[newIndexC*3];
            positions[numPositions++] = positions[newIndexC*3+1];
            positions[numPositions++] = positions[newIndexC*3+2];

            normals[numNormals++] = normals[newIndexA*3];
            normals[numNormals++] = normals[newIndexA*3+1];
            normals[numNormals++] = normals[newIndexA*3+2];

            normals[numNormals++] = normals[newIndexB*3];
            normals[numNormals++] = normals[newIndexB*3+1];
            normals[numNormals++] = normals[newIndexB*3+2];

            normals[numNormals++] = normals[newIndexC*3];
            normals[numNormals++] = normals[newIndexC*3+1];
            normals[numNormals++] = normals[newIndexC*3+2];

            colors[numColors++] = colors[newIndexA*3];
            colors[numColors++] = colors[newIndexA*3+1];
            colors[numColors++] = colors[newIndexA*3+2];

            colors[numColors++] = colors[newIndexB*3];
            colors[numColors++] = colors[newIndexB*3+1];
            colors[numColors++] = colors[newIndexB*3+2];

            colors[numColors++] = colors[newIndexC*3];
            colors[numColors++] = colors[newIndexC*3+1];
            colors[numColors++] = colors[newIndexC*3+2];

            uvs[numUvs++] = uvs[newIndexA*2];
            uvs[numUvs++] = uvs[newIndexA*2+1];

            uvs[numUvs++] = uvs[newIndexB*2];
            uvs[numUvs++] = uvs[newIndexB*2+1];

            uvs[numUvs++] = uvs[newIndexC*2];
            uvs[numUvs++] = uvs[newIndexC*2+1];

            ids[numIds++] = ids[newIndexA];

            ids[numIds++] = ids[newIndexB];

            ids[numIds++] = ids[newIndexC];
          }
        }
        std::cerr << "decimate 2.2 " << quantization << " " << numPositions << " " << points.size() << std::endl;
        break;
      } else {
        quantization *= 2.0f;
        continue;
      }
    }
  }

  std::cerr << "decimate 3 " << quantization << " " << numPositions << std::endl;

  // minTris = (float)std::min(minTris, (float)(numPositions/9));

  std::cerr << "decimate 4 " << quantization << " " << numPositions << std::endl;

  numFaces = numPositions/3;
  for (size_t i = 0; i < numFaces; i++) {
    faces[i] = i;
  }

  std::cerr << "decimate 5 " << quantization << " " << numPositions << std::endl;

  /* size_t target_index_count = ((size_t)minTris)*3;
  std::vector<unsigned int> facesTmp;
  for (size_t i = 0; i < numFaces; i += 3) {
    unsigned int &a = *(unsigned int *)&positions[faces[i]*3];
    unsigned int &b = *(unsigned int *)&positions[faces[i+1]*3];
    unsigned int &c = *(unsigned int *)&positions[faces[i+2]*3];

    if (a != b || a != c) {
      facesTmp.push_back(faces[i]);
      facesTmp.push_back(faces[i+1]);
      facesTmp.push_back(faces[i+2]);
    }
  }

  std::cerr << "decimate 6 " << quantization << " " << numPositions << " " << facesTmp.size() << std::endl;

  numFaces = meshopt_simplify(faces, facesTmp.data(), facesTmp.size(), positions, numPositions, 3*sizeof(float), target_index_count, target_error); */

  std::cerr << "decimate 7 " << quantization << " " << numPositions << std::endl;

  for (size_t i = 0; i < numPositions; i += 9) {
    float *a = &positions[i];
    float *b = &positions[i+3];
    float *c = &positions[i+6];

    float cb[3];
    subVectors(cb, c, b);
    float ab[3];
    subVectors(ab, a, b);
    crossVectors(cb, cb, ab);

    float length = std::sqrt(cb[0] * cb[0] + cb[1] * cb[1] + cb[2] * cb[2]);
    cb[0] /= length;
    cb[1] /= length;
    cb[2] /= length;

    normals[ i ] = cb[0];
    normals[ i + 1 ] = cb[1];
    normals[ i + 2 ] = cb[2];

    normals[ i + 3 ] = cb[0];
    normals[ i + 4 ] = cb[1];
    normals[ i + 5 ] = cb[2];

    normals[ i + 6 ] = cb[0];
    normals[ i + 7 ] = cb[1];
    normals[ i + 8 ] = cb[2];
  }

  std::cerr << "decimate 8 " << quantization << " " << numPositions << std::endl;
}