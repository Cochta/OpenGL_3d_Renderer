#pragma once
#include <GL/glew.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <texture_manager.h>

#include <assimp/Importer.hpp>
#include <string_view>
#include <vector>

class Mesh {
 public:
  Mesh() = default;
  void SetTriangle();
  void SetQuad(float scale = 1);
  void SetCube(float scale = 1, glm::vec2 factor = glm::vec2(1, 1));
  void SetSphere();
  std::vector<float> vertices_;
  std::vector<float> tex_coord_;
  std::vector<float> normals_;
  std::vector<float> tangents_;
  std::vector<float> bitangents_;

  std::vector<GLuint> indices_;

  GLuint vao_;
  std::vector<GLuint> vbo_;
  GLuint ebo_ = 0;

  void Draw();
  void clear();
  void BindVBO(int index, std::vector<float> elements, int size);
};
class Model {
 public:
  GLuint albedo = 0;
  GLuint normal = 0;
  GLuint metallic = 0;
  GLuint ao = 0;
  GLuint roughness = 0;

 private:
  TextureManager tm_;

  std::vector<Mesh> meshes_;
  std::string dir_path_;

 public:
  void Load(std::string_view path, bool flip = false);

  void Draw();
  void Clear();

 private:
  void ProcessNode(aiNode* node, const aiScene* scene);
  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
