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

bool operator==(const vec3f &a, const vec3f &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}
struct Hash {
   size_t operator() (const vec3f &a) const {
     return (size_t)a.x ^ (size_t)a.y ^ (size_t)a.z;
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
  size_t total_indices;
  for (;;) {
    for (size_t i = 0; i < numPositions; i++) {
      positions[i] = std::floor(positions[i]/quantization)*quantization;
    }

    {
      std::unordered_set<vec3f, Hash> points;
      for (size_t i = 0; i < numPositions; i += 3) {
        points.insert(vec3f{
          positions[i],
          positions[i+1],
          positions[i+2],
        });
      }
      if (points.size() < 500000) {
        break;
      } else {
        quantization *= 2.0f;
        continue;
      }
    }
  }

  for (size_t i = 0; i < numPositions; i += 9) {
    float *a = &positions[i];
    float *b = &positions[i+3];
    float *c = &positions[i+6];

    float cb[3];
    subVectors(cb, c, b);
    float ab[3];
    subVectors(ab, a, b);
    crossVectors(cb, cb, ab);

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
  for (size_t i = 0; i < numPositions; i += 3) {
    float *normal = &normals[i];
    float length = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    normal[0] /= length;
    normal[1] /= length;
    normal[2] /= length;
  }

  {
    total_indices = numPositions/3;
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
        };
      }

      total_vertices = meshopt_generateVertexRemap(remap.data(), NULL, total_indices, vertices.data(), total_indices, sizeof(Vertex));
    }

    meshopt_remapVertexBuffer(positions, positions, total_indices, sizeof(float) * 3, &remap[0]);
    numPositions = total_vertices * 3;

    meshopt_remapVertexBuffer(normals, normals, total_indices, sizeof(float) * 3, &remap[0]);
    numNormals = total_vertices * 3;

    meshopt_remapVertexBuffer(colors, colors, total_indices, sizeof(float) * 3, &remap[0]);
    numColors = total_vertices * 3;

    meshopt_remapVertexBuffer(uvs, uvs, total_indices, sizeof(float) * 2, &remap[0]);
    numUvs = total_vertices * 2;

    meshopt_remapVertexBuffer(ids, ids, total_indices, sizeof(unsigned int), &remap[0]);
    numIds = total_vertices;

    meshopt_remapIndexBuffer(faces, NULL, total_indices, &remap[0]);
    numFaces = total_indices;
  }
  /* size_t total_indices = numFaces;
  size_t total_triangles = total_indices/3;
  size_t target_tri_count = std::min((size_t)minTris, total_triangles);
  if (target_tri_count < total_triangles) {
    Simplify::vertices = std::vector<Simplify::Vertex>(numPositions/3);
    for (int i = 0; i < Simplify::vertices.size(); i++) {
      Simplify::Vertex v;
      v.p.x = positions[i*3];
      v.p.y = positions[i*3+1];
      v.p.z = positions[i*3+2];
      Simplify::vertices[i] = v;
    }
    Simplify::triangles = std::vector<Simplify::Triangle>(total_triangles);
    for (int i = 0; i < total_indices; i += 3) {
      Simplify::Triangle t;
      t.v[0] = faces[i];
      t.v[1] = faces[i+1];
      t.v[2] = faces[i+2];
      t.attr |= Simplify::Attributes::TEXCOORD | Simplify::Attributes::NORMAL | Simplify::Attributes::COLOR;
      t.cs[0] = vec3f{
        colors[faces[i]*3],
        colors[faces[i]*3+1],
        colors[faces[i]*3+2],
      };
      t.cs[1] = vec3f{
        colors[faces[i+1]*3],
        colors[faces[i+1]*3+1],
        colors[faces[i+1]*3+2],
      };
      t.cs[2] = vec3f{
        colors[faces[i+2]*3],
        colors[faces[i+2]*3+1],
        colors[faces[i+2]*3+2],
      };
      t.uvs[0] = vec3f{
        uvs[faces[i]*2],
        uvs[faces[i]*2+1],
        0,
      };
      t.uvs[1] = vec3f{
        uvs[faces[i+1]*2],
        uvs[faces[i+1]*2+1],
        0,
      };
      t.uvs[2] = vec3f{
        uvs[faces[i+2]*2],
        uvs[faces[i+2]*2+1],
        0,
      };
      t.ids[0] = ids[faces[i]];
      t.ids[1] = ids[faces[i+1]];
      t.ids[2] = ids[faces[i+2]];
      Simplify::triangles[i/3] = t;
    }
    Simplify::simplify_mesh(target_tri_count, aggressiveness, base, iterationOffset);

    for (size_t i = 0; i < Simplify::vertices.size(); i++) {
      Simplify::Vertex &v = Simplify::vertices[i];
      positions[i*3] = v.p.x;
      positions[i*3+1] = v.p.y;
      positions[i*3+2] = v.p.z;
    }
    numPositions = Simplify::vertices.size()*3;
    numNormals = Simplify::vertices.size()*3;
    numColors = Simplify::vertices.size()*3;
    numUvs = Simplify::vertices.size()*2;
    numIds = Simplify::vertices.size();
    for (int i = 0; i < Simplify::triangles.size(); i++) {
      Simplify::Triangle &t = Simplify::triangles[i];

      for (int j = 0; j < 3; j++) {
        int vi = t.v[j];
        faces[i*3+j] = vi;

        vec3f &n = t.n;
        normals[vi*3] = n.x;
        normals[vi*3+1] = n.y;
        normals[vi*3+2] = n.z;

        vec3f &c = t.cs[j];
        colors[vi*3] = c.x;
        colors[vi*3+1] = c.y;
        colors[vi*3+2] = c.z;

        vec3f &u = t.uvs[j];
        uvs[vi*2] = u.x;
        uvs[vi*2+1] = u.y;

        ids[vi] = t.ids[j];
      }
    }
    numFaces = Simplify::triangles.size()*3;

    Simplify::vertices = decltype(Simplify::vertices)();
    Simplify::triangles = decltype(Simplify::triangles)();
    Simplify::refs = decltype(Simplify::refs)(); */

    size_t target_index_count = std::min((size_t)(minTris*3), (size_t)total_indices);
    std::vector<unsigned int> facesTmp(numFaces);
    memcpy(facesTmp.data(), faces, numFaces * sizeof(unsigned int));
    numFaces = meshopt_simplify(faces, facesTmp.data(), facesTmp.size(), positions, numPositions, 3*sizeof(float), target_index_count, target_error);
    /* if (numFaces > target_index_count) {
      facesTmp = std::vector<unsigned int>(numFaces);
      memcpy(facesTmp.data(), faces, numFaces * sizeof(unsigned int));
      numFaces = meshopt_simplifySloppy(faces, facesTmp.data(), facesTmp.size(), positions, numPositions, 3*sizeof(float), target_index_count);
    } */
    // facesTmp = std::vector<unsigned int>();

    /* const unsigned int oldNumFaces = numFaces;
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
    } */
  // }
}