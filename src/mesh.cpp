#include "mesh.h"

Mesh::Mesh(MeshType type) {
  switch (type) {
    case MeshType::Triangle:
      SetTriangle();
      break;
    case MeshType::Quad:
      SetQuad();
      break;
    case MeshType::Quad1:
      SetQuad(2);
      break;
    case MeshType::Cube:
      SetCube();
      break;
    case MeshType::Cube05:
      SetCube(0.5);
      break;
    case MeshType::Cube30:
      SetCube(30);
      break;
    case MeshType::Sphere:
      SetSphere();
      break;
    case MeshType::Model:
      break;
    default:
      break;
  }
}
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

  for (int i = 0; i < 3; ++i) {
    vbo_.emplace_back();
  }
  glGenBuffers(3, &vbo_[0]);
  BindVBO(0, vertices_, 3);
  BindVBO(1, tex_coord_, 2);
  BindVBO(2, normals_, 3);

  glGenBuffers(1, &ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
               indices_.data(), GL_STATIC_DRAW);
}

void Mesh::SetCube(float scale) {
  float size = 1.f * scale;
  vertices_ = {size,  size,  size,  -size, size,  size,  // front
               -size, -size, size,  size,  -size, size,

               size,  size,  -size, -size, size,  -size,  // up
               -size, size,  size,  size,  size,  size,

               size,  size,  -size, size,  -size, -size,  // back
               -size, -size, -size, -size, size,  -size,

               size,  -size, -size, size,  -size, size,  // down
               -size, -size, size,  -size, -size, -size,

               size,  size,  -size, size,  size,  size,  // right
               size,  -size, size,  size,  -size, -size,

               -size, size,  -size, -size, -size, -size,  // left
               -size, size,  size,  -size, -size, size};
  indices_ = {
      0,  1,  3,  1,  2,  3,   // Front face
      4,  5,  7,  5,  6,  7,   // Up face.
      8,  9,  11, 11, 9,  10,  // Back face.
      12, 13, 15, 15, 13, 14,  // Down face.
      16, 17, 19, 17, 18, 19,  // Right face.
      20, 21, 22, 22, 21, 23,  // Left face.
  };
  if (size < 1) {
    size = 1.0f;
  }
  tex_coord_ = {
      size, size, 0.f,  size, 0.f,  0.f,  size, 0.f,   // front
      size, size, 0.f,  size, 0.f,  0.f,  size, 0.f,   // up
      size, size, size, 0.f,  0.f,  0.f,  0.f,  size,  // back
      size, size, size, 0.f,  0.f,  0.f,  0.f,  size,  // down
      size, size, 0.f,  size, 0.f,  0.f,  size, 0.f,   // right
      0.f,  size, 0.f,  0.f,  size, size, size, 0.f,   // left
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

  for (int i = 0; i < 4; ++i) {
    vbo_.emplace_back();
  }
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
}

void Mesh::Draw() {
  glBindVertexArray(vao_);

  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::clear() {
  vertices_.clear();
  tex_coord_.clear();
  normals_.clear();
  tangents_.clear();

  indices_.clear();
}

void Model::Load(std::string_view path, bool gamma, bool flip, bool pbr) {
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

  ProcessNode(scene->mRootNode, scene, gamma, flip, pbr);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, bool gamma,
                        bool flip, bool pbr) {
  meshes_.reserve(node->mNumMeshes);

  // Process all the node's meshes (if any).
  for (std::size_t i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes_.emplace_back(ProcessMesh(mesh, scene, gamma, flip, pbr));
  }

  // Do the same for each of its children.
  for (std::size_t i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene, gamma, flip, pbr);
  }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, bool gamma,
                        bool flip, bool pbr) {
  Mesh my_mesh(MeshType::Model);

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

  if (!pbr) {
    // Process material.
    if (mesh->mMaterialIndex >= 0) {
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

      //  Diffuse maps.
      std::vector<Texture> diffuseMaps = LoadAllTextures(
          material, aiTextureType_DIFFUSE, "texture_diffuse", flip);
      my_mesh.textures_.insert(my_mesh.textures_.end(), diffuseMaps.begin(),
                               diffuseMaps.end());

      // Specular maps.
      std::vector<Texture> specularMaps = LoadAllTextures(
          material, aiTextureType_SPECULAR, "texture_specular", flip);
      my_mesh.textures_.insert(my_mesh.textures_.end(), specularMaps.begin(),
                               specularMaps.end());

      // Specular maps.
      std::vector<Texture> normalMaps = LoadAllTextures(
          material, aiTextureType_HEIGHT, "texture_normal", flip);
      my_mesh.textures_.insert(my_mesh.textures_.end(), normalMaps.begin(),
                               normalMaps.end());
    }
  }

  glGenVertexArrays(1, &my_mesh.vao_);
  glBindVertexArray(my_mesh.vao_);

  for (int i = 0; i < 5; ++i) {
    my_mesh.vbo_.emplace_back();
  }
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

std::vector<Texture> Model::LoadAllTextures(aiMaterial* mat, aiTextureType type,
                                            std::string type_name, bool flip) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);

    bool skip = false;
    for (unsigned int j = 0; j < tm_.loaded_textures.size(); j++) {
      if (dir_path_ + tm_.loaded_textures[j].path == dir_path_ + str.data) {
        textures.push_back(tm_.loaded_textures[j]);
        skip = true;
        break;
      }
    }

    // If texture hasn't been loaded already, load it.
    if (!skip) {
      Texture texture;
      std::string texture_path = str.data;
      texture.id = tm_.LoadTexture(dir_path_ + '/' + texture_path, flip);
      texture.type = type_name;
      texture.path = texture_path;
      textures.push_back(texture);
      tm_.loaded_textures.push_back(texture);
    }
  }
  return textures;
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
}
