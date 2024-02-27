#include "mesh.h"

#ifdef TRACY_ENABLE
#include <TracyC.h>

#include <Tracy.hpp>
#endif

void Mesh::BindVBO(int index, std::vector<float> elements, int size) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[index]);
  glBufferData(GL_ARRAY_BUFFER, elements.size() * sizeof(float),
               elements.data(), GL_STATIC_DRAW);

  // Associate vertex attributes with the VBO
  glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, size * sizeof(float),
                        (void*)0);
  glEnableVertexAttribArray(index);
}

void Mesh::SetTriangle() {
  vertices_ = {1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0};
  indices_ = {0, 1, 2};
}
void Mesh::SetQuad(float scale) {
  float size = 0.5f * scale;
  vertices_ = {
      -size, size,  0.0,  // Top-let
      size,  size,  0.0,  // Top-right
      size,  -size, 0.0,  // Bottom-right
      -size, -size, 0.0,  // Bottom-let
  };
  indices_ = {0, 3, 2, 0, 2, 1};
  tex_coord_ = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};

  glGenVertexArrays(1, &vao_);  // create
  glBindVertexArray(vao_);

  vbo_.resize(3);

  glGenBuffers(3, &vbo_[0]);
  BindVBO(0, vertices_, 3);
  BindVBO(1, tex_coord_, 2);
  BindVBO(2, normals_, 3);

  glGenBuffers(1, &ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
               indices_.data(), GL_STATIC_DRAW);
}

void Mesh::SetCube(float scale, glm::vec2 factor) {
  vertices_ = {scale,  scale,  scale,  -scale, scale,  scale,  // front
               -scale, -scale, scale,  scale,  -scale, scale,

               scale,  scale,  -scale, -scale, scale,  -scale,  // up
               -scale, scale,  scale,  scale,  scale,  scale,

               scale,  scale,  -scale, scale,  -scale, -scale,  // back
               -scale, -scale, -scale, -scale, scale,  -scale,

               scale,  -scale, -scale, scale,  -scale, scale,  // down
               -scale, -scale, scale,  -scale, -scale, -scale,

               scale,  scale,  -scale, scale,  scale,  scale,  // right
               scale,  -scale, scale,  scale,  -scale, -scale,

               -scale, scale,  -scale, -scale, -scale, -scale,  // left
               -scale, scale,  scale,  -scale, -scale, scale};
  indices_ = {
      0,  1,  3,  1,  2,  3,   // Front face
      4,  5,  7,  5,  6,  7,   // Up face.
      8,  9,  11, 11, 9,  10,  // Back face.
      12, 13, 15, 15, 13, 14,  // Down face.
      16, 17, 19, 17, 18, 19,  // Right face.
      20, 21, 22, 22, 21, 23,  // Left face.
  };
  if (scale < 1) {
    scale = 1.0f;
  }
  float x = scale * factor.x;
  float y = scale * factor.y;
  tex_coord_ = {
      scale, y,     0.f,   y,     0.f, 0.f, x,     0.f,    // front
      scale, scale, 0.f,   scale, 0.f, 0.f, scale, 0.f,    // up
      x,     y,     x,     0.f,   0.f, 0.f, 0.f,   y,      // back
      scale, scale, scale, 0.f,   0.f, 0.f, 0.f,   scale,  // down
      x,     y,     0.f,   y,     0.f, 0.f, x,     0.f,    // right
      0.f,   y,     0.f,   0.f,   x,   y,   x,     0.f,    // left
  };
  normals_ = {
      0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  // front
      0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,

      0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  // up
      0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,  // back,
      0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,

      0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  // down
      0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,

      1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // right
      1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  // left
      -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,
  };
  tangents_ = {
      1, 0,  -0, 1, 0,  0,  1, 0,  0,  1, 0,  0,  1,  -0, -0, 1, 0,  0,
      1, 0,  0,  1, 0,  0,  1, -0, -0, 1, -0, -0, 1,  -0, -0, 1, -0, -0,
      1, -0, -0, 1, -0, -0, 1, -0, -0, 1, -0, -0, -0, 0,  -1, 0, 0,  -1,
      0, 0,  -1, 0, 0,  -1, 0, 0,  1,  0, 0,  1,  0,  0,  1,  0, 0,  1,
  };

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  vbo_.resize(4);

  glGenBuffers(4, &vbo_[0]);
  BindVBO(0, vertices_, 3);
  BindVBO(1, tex_coord_, 2);
  BindVBO(2, normals_, 3);
  BindVBO(3, tangents_, 3);

  glGenBuffers(1, &ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
               indices_.data(), GL_STATIC_DRAW);
}

void Mesh::SetSphere() {
  static unsigned int indexCount;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uv;
  std::vector<glm::vec3> normals;

  const unsigned int X_SEGMENTS = 64;
  const unsigned int Y_SEGMENTS = 64;
  const float PI = 3.14159265359f;
  for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
      float xSegment = (float)x / (float)X_SEGMENTS;
      float ySegment = (float)y / (float)Y_SEGMENTS;
      float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
      float yPos = std::cos(ySegment * PI);
      float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

      positions.push_back(glm::vec3(xPos, yPos, zPos));
      uv.push_back(glm::vec2(xSegment, ySegment));
      normals.push_back(glm::vec3(xPos, yPos, zPos));
      
      tangents_.push_back(std::sin(xSegment * 2.0f * PI) * std::cos(ySegment * PI));
      tangents_.push_back(std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI));
      tangents_.push_back(std::cos(xSegment * 2.0f * PI));
    }
  }

  bool oddRow = false;
  for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
    if (!oddRow)  // even rows: y == 0, y == 2; and so on
    {
      for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
        indices_.push_back(y * (X_SEGMENTS + 1) + x);
        indices_.push_back((y + 1) * (X_SEGMENTS + 1) + x);
      }
    } else {
      for (int x = X_SEGMENTS; x >= 0; --x) {
        indices_.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        indices_.push_back(y * (X_SEGMENTS + 1) + x);
      }
    }
    oddRow = !oddRow;
  }
  indexCount = static_cast<unsigned int>(indices_.size());

  for (unsigned int i = 0; i < positions.size(); ++i) {
    vertices_.push_back(positions[i].x);
    vertices_.push_back(positions[i].y);
    vertices_.push_back(positions[i].z);
    if (normals.size() > 0) {
      normals_.push_back(normals[i].x);
      normals_.push_back(normals[i].y);
      normals_.push_back(normals[i].z);
    }
    if (uv.size() > 0) {
      tex_coord_.push_back(uv[i].x);
      tex_coord_.push_back(uv[i].y);
    }
  }

  glGenVertexArrays(1, &vao_);  // create
  glBindVertexArray(vao_);

  vbo_.resize(4);

  glGenBuffers(4, &vbo_[0]);
  BindVBO(0, vertices_, 3);
  BindVBO(1, tex_coord_, 2);
  BindVBO(2, normals_, 3);
  BindVBO(3, tangents_, 3);

  glGenBuffers(1, &ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
               indices_.data(), GL_STATIC_DRAW);
}

void Mesh::Draw(bool is_sphere) {
  glBindVertexArray(vao_);

  glDrawElements(!is_sphere ? GL_TRIANGLES : GL_TRIANGLE_STRIP, indices_.size(),
                 GL_UNSIGNED_INT, 0);
}

void Mesh::clear() {
  vertices_.clear();
  tex_coord_.clear();
  normals_.clear();
  tangents_.clear();

  indices_.clear();
}

void Material::Set()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, albedo);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, metallic);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, roughness);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, ao);
}

void Material::Clear()
{
  albedo = 0;
  normal = 0;
  metallic = 0;
  ao = 0;
  roughness = 0;
}

void Model::Load(std::string_view path, bool flip) {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  Assimp::Importer import;
  auto flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
  if (flip) {
    flags = flags | aiProcess_FlipUVs;
  }
  const aiScene* scene = import.ReadFile(path.data(), flags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << '\n';
    return;
  }

  dir_path_ = path.substr(0, path.find_last_of('/'));

  ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
  meshes_.reserve(node->mNumMeshes);

  // Process all the node's meshes (if any).
  for (std::size_t i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes_.emplace_back(ProcessMesh(mesh, scene));
  }

  // Do the same for each of its children.
  for (std::size_t i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
  Mesh my_mesh;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    my_mesh.vertices_.push_back(mesh->mVertices[i].x);
    my_mesh.vertices_.push_back(mesh->mVertices[i].y);
    my_mesh.vertices_.push_back(mesh->mVertices[i].z);

    my_mesh.normals_.push_back(mesh->mNormals[i].x);
    my_mesh.normals_.push_back(mesh->mNormals[i].y);
    my_mesh.normals_.push_back(mesh->mNormals[i].z);

    my_mesh.tangents_.push_back(mesh->mTangents[i].x);
    my_mesh.tangents_.push_back(mesh->mTangents[i].y);
    my_mesh.tangents_.push_back(mesh->mTangents[i].z);

    my_mesh.bitangents_.push_back(mesh->mBitangents[i].x);
    my_mesh.bitangents_.push_back(mesh->mBitangents[i].y);
    my_mesh.bitangents_.push_back(mesh->mBitangents[i].z);

    // If the mesh contains texture coordinates, stores it.
    if (mesh->mTextureCoords[0]) {
      my_mesh.tex_coord_.push_back(mesh->mTextureCoords[0][i].x);
      my_mesh.tex_coord_.push_back(mesh->mTextureCoords[0][i].y);
    } else {
      my_mesh.tex_coord_.push_back(0);
      my_mesh.tex_coord_.push_back(0);
    }
  }

  // Process indices (each faces has a number of indices).
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];

    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      my_mesh.indices_.push_back(face.mIndices[j]);
    }
  }

  glGenVertexArrays(1, &my_mesh.vao_);
  glBindVertexArray(my_mesh.vao_);

  my_mesh.vbo_.resize(5);

  glGenBuffers(4, &my_mesh.vbo_[0]);
  my_mesh.BindVBO(0, my_mesh.vertices_, 3);
  my_mesh.BindVBO(1, my_mesh.tex_coord_, 2);
  my_mesh.BindVBO(2, my_mesh.normals_, 3);
  my_mesh.BindVBO(3, my_mesh.tangents_, 3);
  my_mesh.BindVBO(4, my_mesh.bitangents_, 3);

  glGenBuffers(1, &my_mesh.ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_mesh.ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               my_mesh.indices_.size() * sizeof(GLuint),
               my_mesh.indices_.data(), GL_STATIC_DRAW);
  return my_mesh;
}

void Model::Draw() {
  for (auto& mesh : meshes_) {
    mesh.Draw();
  }
}

void Model::Clear() {
  for (auto& mesh : meshes_) {
    mesh.clear();
  }
  mat.Clear();
}
