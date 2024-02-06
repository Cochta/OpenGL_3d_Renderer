#pragma once
#include <GL/glew.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <texture_manager.h>

#include <assimp/Importer.hpp>
#include <string_view>
#include <vector>

enum class MeshType {
  Triangle,
  Quad,
  Quad1,
  Cube,
  Cube05,
  Cube30,
  Sphere,
  Model
};

class Mesh {
 public:
  Mesh() = delete;
  Mesh(MeshType type);

  ~Mesh() { clear(); }

  std::vector<float> vertices_;
  std::vector<float> tex_coord_;
  std::vector<float> normals_;
  std::vector<float> tangents_;
  std::vector<float> bitangents_;

  std::vector<GLuint> indices_;

  std::vector<Texture> textures_;

  GLuint vao_;
  std::vector<GLuint> vbo_;
  GLuint ebo_ = 0;
  void BindVBO(int index, std::vector<float> elements, int size);

 private:
  void SetTriangle();
  void SetQuad(float scale = 1);
  void SetCube(float scale = 1);
  void SetSphere();

 public:
  void Draw();
  void clear();
};
class Model {
 public:
  GLuint albedo_ = 0;
  GLuint normal_ = 0;
  GLuint metallic_ = 0;
  GLuint ao_ = 0;
  GLuint roughness_ = 0;

  TextureManager tm_;

  // Mesh mesh_ = Mesh(MeshType::Model);

  std::vector<Mesh> meshes_;
  std::string dir_path_;

  void Load(std::string_view path, bool gamma = false, bool flip = false,
            bool pbr = false);

  void ProcessNode(aiNode* node, const aiScene* scene, bool gamma = false,
                   bool flip = false, bool pbr = false);
  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, bool gamma = false,
                   bool flip = false, bool pbr = false);
  std::vector<Texture> LoadAllTextures(aiMaterial* mat, aiTextureType type,
                                       std::string type_name,
                                       bool flip = false);

  void Draw();
  void Clear();
};
