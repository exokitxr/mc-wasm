#include "uv.h"

/* bool ProgressCallback(xatlas::ProgressCategory::Enum category, int progress, void *userData) {
  std::cerr << xatlas::StringForEnum(category) << std::endl;
  return true;
} */

void uvParameterize(float *positions, unsigned int numPositions, float *normals, unsigned int numNormals, unsigned int *faces, unsigned int numFaces, float *outPositions, unsigned int &numOutPositions, float *outNormals, unsigned int &numOutNormals, unsigned int *outFaces, float *uvs, unsigned int &numUVs) {
  xatlas::Atlas *atlas = xatlas::Create();
  // xatlas::SetProgressCallback(atlas, ProgressCallback, nullptr);

  // uint32_t totalVertices = 0, totalFaces = 0;
  {
    // std::cout << "uv 4" << std::endl;

    xatlas::MeshDecl meshDecl;
    meshDecl.vertexCount = numPositions / 3;
    meshDecl.vertexPositionData = positions;
    meshDecl.vertexPositionStride = sizeof(float) * 3;
    meshDecl.indexCount = 0;
    meshDecl.indexData = nullptr;
    meshDecl.indexCount = numFaces;
    meshDecl.indexData = faces;
    meshDecl.indexFormat = xatlas::IndexFormat::UInt32;
    // std::cout << "input vertex count " << meshDecl.vertexCount << std::endl;
    xatlas::AddMeshError::Enum error = xatlas::AddMesh(atlas, meshDecl, 1);
    if (error != xatlas::AddMeshError::Success) {
      xatlas::Destroy(atlas);
      std::cerr << "Error adding mesh " << xatlas::StringForEnum(error) << std::endl;
      return;
    }
    // std::cout << "uv 6" << std::endl;
    // totalVertices += meshDecl.vertexCount;
    // totalFaces += meshDecl.indexCount / 3;
  }

  xatlas::Generate(atlas);

  // std::cout << "uv 7" << std::endl;

  // std::cout << "mesh count " << atlas->chartCount << " " << atlas->atlasCount << " " << atlas->width << " " << atlas->height << " " << atlas->meshCount << std::endl;

  numOutPositions = 0;
  numOutNormals = 0;
  numUVs = 0;
  for (uint32_t i = 0; i < atlas->meshCount; i++) {
    const xatlas::Mesh &mesh = atlas->meshes[i];
    for (uint32_t v = 0; v < mesh.vertexCount; v++) {
      const xatlas::Vertex &vertex = mesh.vertexArray[v];

      memcpy(&outPositions[numOutPositions], &positions[vertex.xref * 3], 3*sizeof(float));
      numOutPositions += 3;

      memcpy(&outNormals[numOutNormals], &normals[vertex.xref * 3], 3*sizeof(float));
      numOutNormals += 3;

      uvs[numUVs++] = vertex.uv[0] / atlas->width;
      uvs[numUVs++] = vertex.uv[1] / atlas->height;
    }
    memcpy(outFaces, mesh.indexArray, mesh.indexCount*sizeof(unsigned int));
  }

  xatlas::Destroy(atlas);

  // std::cout << "vertex count " << numUVs << std::endl;
}