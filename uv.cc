#include "uv.h"

/* bool ProgressCallback(xatlas::ProgressCategory::Enum category, int progress, void *userData) {
  std::cerr << xatlas::StringForEnum(category) << std::endl;
  return true;
} */

void uvParameterize(float *positions, unsigned int numPositions, unsigned int *faces, unsigned int numFaces, float *uvs, unsigned int &numUVs) {
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

  numUVs = 0;
  for (uint32_t i = 0; i < atlas->meshCount; i++) {
    const xatlas::Mesh &mesh = atlas->meshes[i];
    for (uint32_t v = 0; v < mesh.vertexCount; v++) {
      const xatlas::Vertex &vertex = mesh.vertexArray[v];
      // const float *pos = &positions[vertex.xref * 3];
      // std::cout << "v " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
      /* if (!shapes[i].mesh.normals.empty()) {
        const float *normal = &shapes[i].mesh.normals[vertex.xref * 3];
        fprintf(file, "vn %g %g %g\n", normal[0], normal[1], normal[2]);
      } */
      // std::cout << "vt " << (vertex.uv[0] / atlas->width) << " " << (vertex.uv[1] / atlas->height) << std::endl;
      uvs[numUVs++] = vertex.uv[0] / atlas->width;
      uvs[numUVs++] = vertex.uv[1] / atlas->height;
    }
    // std::cout << "o " << shapes[i].name.c_str() << std::endl;
    /* for (uint32_t f = 0; f < mesh.indexCount; f += 3) {
      fprintf(file, "f ");
      for (uint32_t j = 0; j < 3; j++) {
        const uint32_t index = firstVertex + mesh.indexArray[f + j] + 1; // 1-indexed
        fprintf(file, "%d/%d/%d%c", index, index, index, j == 2 ? '\n' : ' ');
      }
    } */
    // numUVs += mesh.vertexCount * 2;
  }

  xatlas::Destroy(atlas);

  // std::cout << "vertex count " << numUVs << std::endl;
}