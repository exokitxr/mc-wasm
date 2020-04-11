#include "cut.h"
#include "Simplify.h"

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
  float minTris,
  float aggressiveness,
  float base,
  int iterationOffset,
  unsigned int *faces,
  unsigned int &numFaces
) {
  {
    size_t total_indices = numPositions/3;
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

    meshopt_remapVertexBuffer(positions, positions, total_indices, sizeof(float) * 3, &remap[0]);
    numPositions = total_vertices * 3;

    meshopt_remapVertexBuffer(normals, normals, total_indices, sizeof(float) * 3, &remap[0]);
    numNormals = total_vertices * 3;

    meshopt_remapVertexBuffer(colors, colors, total_indices, sizeof(float) * 3, &remap[0]);
    numColors = total_vertices * 3;

    meshopt_remapVertexBuffer(uvs, uvs, total_indices, sizeof(float) * 2, &remap[0]);
    numUvs = total_vertices * 2;

    meshopt_remapIndexBuffer(faces, NULL, total_indices, &remap[0]);
    numFaces = total_indices;
  }






  

  Simplify::vertices = std::vector<Simplify::Vertex>(numPositions/3);
  for (int i = 0; i < Simplify::vertices.size(); i++) {
    Simplify::Vertex v;
    v.p.x = positions[i*3];
    v.p.y = positions[i*3+1];
    v.p.z = positions[i*3+2];
    Simplify::vertices[i] = v;
  }
  size_t total_indices = numFaces;
  Simplify::triangles = std::vector<Simplify::Triangle>(total_indices/3);
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
    Simplify::triangles[i/3] = t;
  }
  // int v[3];double err[4];int deleted,dirty,attr;vec3f n;vec3f uvs[3];int material; };
  size_t target_tri_count = std::min((size_t)minTris, Simplify::triangles.size());
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
  numFaces = Simplify::vertices.size()*2;
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
    }
  }
  numFaces = Simplify::triangles.size()*3;

  Simplify::vertices.clear();
  Simplify::triangles.clear();
  Simplify::refs.clear();

  /* 

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
  } */
}