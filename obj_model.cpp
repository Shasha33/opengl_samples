#include "obj_model.h"

#include <glm/glm.hpp>

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <GL/glew.h>


class obj_model_t : public IObjModel
{
public:
  obj_model_t(char const * filename);
  ~obj_model_t() = default;

  void draw() override;
private:
   int num_vertices_ = 0;
   GLuint vao_ = 0;
   GLuint vbo_ = 0;
};


obj_model_t::obj_model_t(char const * filename)
{
   spdlog::info("Loading model: {}", filename);

   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;

   auto parent = fs::absolute(fs::path(filename)).parent_path();
   std::string err;
   const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, parent.u8string().c_str(), true);

   if (!ret)
      throw std::runtime_error(fmt::format("Model load error: {}", err));

   std::vector<glm::vec3> vertices;

   for (auto const & shape : shapes)
   {
      for (int i = 0; i < shape.mesh.indices.size(); ++i)
      {
         const auto vert_x = attrib.vertices[3 * shape.mesh.indices[i].vertex_index];
         const auto vert_y = attrib.vertices[3 * shape.mesh.indices[i].vertex_index + 1];
         const auto vert_z = attrib.vertices[3 * shape.mesh.indices[i].vertex_index + 2];
         vertices.push_back({vert_x, vert_y, vert_z});

         const auto norm_x = attrib.normals[3 * shape.mesh.indices[i].normal_index];
         const auto norm_y = attrib.normals[3 * shape.mesh.indices[i].normal_index + 1];
         const auto norm_z = attrib.normals[3 * shape.mesh.indices[i].normal_index + 2];
         vertices.push_back({norm_x, norm_y, norm_z});
      }
   }
   num_vertices_ = vertices.size();

   glGenVertexArrays(1, &vao_);
   glGenBuffers(1, &vbo_);
   glBindVertexArray(vao_);
   glBindBuffer(GL_ARRAY_BUFFER, vbo_);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
}

void obj_model_t::draw()
{
   glBindVertexArray(vao_);
   glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
   glBindVertexArray(0);
}

std::shared_ptr<IObjModel> create_model(char const * filename)
{
  return std::static_pointer_cast<IObjModel>(std::make_shared<obj_model_t>(filename));
}