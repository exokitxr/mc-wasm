#include "uv.h"

bool ProgressCallback(xatlas::ProgressCategory::Enum category, int progress, void *userData) {
  std::cerr << xatlas::StringForEnum(category) << std::endl;
  return true;
}

void uvParameterize(float *positions, unsigned int numPositions) {
  xatlas::Atlas *atlas = xatlas::Create();
  xatlas::SetProgressCallback(atlas, ProgressCallback, nullptr);

  uint32_t totalVertices = 0, totalFaces = 0;
  {
    xatlas::MeshDecl meshDecl;
    meshDecl.vertexCount = numPositions / 3;
    meshDecl.vertexPositionData = positions;
    meshDecl.vertexPositionStride = sizeof(float) * 3;
    meshDecl.indexCount = 0;
    meshDecl.indexData = nullptr;
    /* meshDecl.indexCount = (uint32_t)objMesh.indices.size();
    meshDecl.indexData = objMesh.indices.data();
    meshDecl.indexFormat = xatlas::IndexFormat::UInt32; */
    xatlas::AddMeshError::Enum error = xatlas::AddMesh(atlas, meshDecl, 1);
    if (error != xatlas::AddMeshError::Success) {
      xatlas::Destroy(atlas);
      std::cerr << "Error adding mesh " << xatlas::StringForEnum(error) << std::endl;
      return;
    }
    totalVertices += meshDecl.vertexCount;
    totalFaces += meshDecl.indexCount / 3;
  }

  uint32_t firstVertex = 0;
  for (uint32_t i = 0; i < atlas->meshCount; i++) {
    const xatlas::Mesh &mesh = atlas->meshes[i];
    for (uint32_t v = 0; v < mesh.vertexCount; v++) {
      const xatlas::Vertex &vertex = mesh.vertexArray[v];
      const float *pos = &positions[vertex.xref * 3];
      std::cout << "v " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
      /* if (!shapes[i].mesh.normals.empty()) {
        const float *normal = &shapes[i].mesh.normals[vertex.xref * 3];
        fprintf(file, "vn %g %g %g\n", normal[0], normal[1], normal[2]);
      } */
      std::cout << "vt " << (vertex.uv[0] / atlas->width) << " " << (vertex.uv[1] / atlas->height) << std::endl;
    }
    // std::cout << "o " << shapes[i].name.c_str() << std::endl;
    /* for (uint32_t f = 0; f < mesh.indexCount; f += 3) {
      fprintf(file, "f ");
      for (uint32_t j = 0; j < 3; j++) {
        const uint32_t index = firstVertex + mesh.indexArray[f + j] + 1; // 1-indexed
        fprintf(file, "%d/%d/%d%c", index, index, index, j == 2 ? '\n' : ' ');
      }
    } */
    firstVertex += mesh.vertexCount;
  }
}