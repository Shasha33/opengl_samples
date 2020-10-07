#pragma optimize("", off)

#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif
// // STB, load images
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>


// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shader.h"

#include "obj_model.h"
#include "torus.h"

double translation[] = { 0.0, 0.0 };
double window_h = 1280, window_w = 720;

glm::vec3 cameraPos;

double cameraStep = 0.01;
double zoom = 1;

void update_camerapos() {
   cameraPos.x = cos(translation[0]) * cos(translation[1]);
   cameraPos.y = sin(translation[1]);
   cameraPos.z = sin(translation[0]) * cos(translation[1]);
   cameraPos *= zoom;
}

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_A) {
      translation[0] -= cameraStep;
   }
   if (key == GLFW_KEY_D) {
      translation[0] += cameraStep;
   }
   if (key == GLFW_KEY_W) {
      translation[1] -= cameraStep;
   }
   if (key == GLFW_KEY_S) {
      translation[1] += cameraStep;
   }

   if (translation[0] > 2*glm::pi<double>()) {
      translation[0] -= 2*glm::pi<double>();
   }
   if (translation[1] > 2*glm::pi<double>()) {
      translation[1] -= 2*glm::pi<double>();
   }
   if (translation[0] < 0) {
      translation[0] += 2*glm::pi<double>();
   }
   if (translation[1] < 0) {
      translation[1] += 2*glm::pi<double>();
   }

   update_camerapos();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   zoom += yoffset / window_h * 5;
   update_camerapos();
}

static void glfw_error_callback(int error, const char *description)
{
   throw std::runtime_error(fmt::format("Glfw Error {}: {}\n", error, description));
}

void create_quad(GLuint &vbo, GLuint &vao, GLuint &ebo)
{
   // create the triangle
   const float vertices[] = {
       -1, 1,
       0, 1,

       -1, -1,
       0, 0,

       1, -1,
       1, 0,

       1, 1,
       1, 1,
   };
   const unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glGenBuffers(1, &ebo);
   glBindVertexArray(vao);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
}

struct render_target_t
{
   render_target_t(int res_x, int res_y);
   ~render_target_t();

   GLuint fbo_;
   GLuint color_, depth_;
   int width_, height_;
};

render_target_t::render_target_t(int res_x, int res_y)
{
   width_ = res_x;
   height_ = res_y;

   glGenTextures(1, &color_);
   glBindTexture(GL_TEXTURE_2D, color_);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, res_x, res_y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

   glGenTextures(1, &depth_);
   glBindTexture(GL_TEXTURE_2D, depth_);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, res_x, res_y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

   glBindTexture(GL_TEXTURE_2D, 0);

   glGenFramebuffers(1, &fbo_);

   glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

   glFramebufferTexture2D(GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D,
      color_,
      0);

   glFramebufferTexture2D(GL_FRAMEBUFFER,
      GL_DEPTH_ATTACHMENT,
      GL_TEXTURE_2D,
      depth_,
      0);

   GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("Framebuffer incomplete");

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

render_target_t::~render_target_t()
{
   glDeleteFramebuffers(1, &fbo_);
   glDeleteTextures(1, &depth_);
   glDeleteTextures(1, &color_);
}

void load_image(GLuint & texture, const char* filename)
{
   int width, height, channels;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *image = stbi_load(filename,
      &width,
      &height,
      &channels,
      STBI_rgb_alpha);

   glGenTextures(1, &texture);  
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
   glGenerateMipmap(GL_TEXTURE_2D);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
   if (!image) {
      printf("%s !\n", filename);
   }

   stbi_image_free(image);
}

void create_skybox(GLuint &vbo, GLuint &vao, GLuint &ebo) {
   float vertices[] = {
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
      1.0f,  1.0f, -1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f,  1.0f
   };

   unsigned size = sizeof(vertices) / sizeof(float); 

   for (auto i = 0; i < size; i++) {
      vertices[i] *= 50;
   }

   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glBindVertexArray(vao);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)(0));
}

unsigned int cubemap_texture() {
   unsigned int texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
   stbi_set_flip_vertically_on_load(false);

   int width, height, nrChannels;
   std::vector<std::string> faces {
    "assets/lightblue/right.png",
    "assets/lightblue/left.png",
    "assets/lightblue/top.png",
    "assets/lightblue/bot.png",
    "assets/lightblue/front.png",
    "assets/lightblue/back.png"
   };


   for (unsigned int i = 0; i < faces.size(); i++) {
      unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
   }

   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

   return texture;
}

int main(int, char **)
{
   try
   {
      // Use GLFW to create a simple window
      glfwSetErrorCallback(glfw_error_callback);
      if (!glfwInit())
         throw std::runtime_error("glfwInit error");

      // GL 3.3 + GLSL 330
      const char *glsl_version = "#version 330";
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

      // Create window with graphics context
      GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
      if (window == NULL)
         throw std::runtime_error("Can't create glfw window");

      glfwMakeContextCurrent(window);
      glfwSwapInterval(1); // Enable vsync

      if (glewInit() != GLEW_OK)
         throw std::runtime_error("Failed to initialize glew");

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      // init shader
      shader_t cubemap_shader("assets/simple-shader.vs", "assets/simple-shader.fs");
      shader_t torus_shader("assets/torus.vs", "assets/torus.fs");
      shader_t ufo_shader("assets/ufo.vs", "assets/ufo.fs");

      GLuint vbo, vao, ebo;
      create_skybox(vbo, vao, ebo);

      unsigned int cubemap = cubemap_texture();

      // Setup GUI context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO();
      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init(glsl_version);
      ImGui::StyleColorsDark();

      auto const start_time = std::chrono::steady_clock::now();

      glfwSetKeyCallback(window, keyboard_callback);
      glfwSetScrollCallback(window, scroll_callback);
      update_camerapos();

      auto torus = Torus(512, 256, 1.0, 0.5);
      torus.initializeData();

      GLuint grass, sand, rock, height_map;
      load_image(grass, "assets/grass.png");
      load_image(sand, "assets/sand_grass.png");
      load_image(rock, "assets/rock.png");
      
      torus.initializeMap(height_map, "assets/height_map.png");

      auto ufo = create_model("assets/ufo.obj");

      while (!glfwWindowShouldClose(window))
      {
         glfwPollEvents();

         // Get windows size
         int display_w, display_h;
         glfwGetFramebufferSize(window, &display_w, &display_h);


         // Gui start new frame
         ImGui_ImplOpenGL3_NewFrame();
         ImGui_ImplGlfw_NewFrame();
         ImGui::NewFrame();
         
         glColorMask(1,1,1,1);
         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         auto point = torus.get_point(translation[1], translation[0]);
         auto tex = torus.get_texture(translation[1], translation[0]);
         auto normal = torus.get_normal(translation[1], translation[0]);
         auto tex_normal = torus.get_map_normal(translation[1], translation[0]);
         auto tex_point = torus.get_map_point(translation[1], translation[0]);
         float height = torus.get_map_height(tex);

         // printf("normal %f %f %f\n", normal.x, normal.y, normal.z);
         // printf("tex_normal %f %f %f\n", tex_normal.x, tex_normal.y, tex_normal.z);
      
         auto model = glm::mat4(1.0);
         auto camera_position = point + normal * 2.0f;
         auto view = glm::lookAt(camera_position * (float) zoom, point, glm::vec3(0, 1, 0));
         auto projection = glm::perspective<float>(glm::radians(90.0), (float)display_w / display_h, 0.1, 100);

         // cubemap
         {
            auto mvp = projection * glm::mat4(glm::mat3(view)) * model;

            glViewport(0, 0, display_w, display_h);
            
            cubemap_shader.use();
            cubemap_shader.set_uniform("u_mvp", glm::value_ptr(mvp));

            glBindVertexArray(vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
         }
         
         // torus
         {
            auto torus_view = view * glm::rotate(model, (float) glm::radians(90.0), glm::vec3(1, 0, 0));
            
            auto model = glm::scale(glm::vec3(1, 1, 1));
            auto mvp = projection * view * model;;
            int u_repeat = 10;
            torus_shader.use();
            torus_shader.set_uniform("u_repeat", u_repeat);
            torus_shader.set_uniform("u_mvp", glm::value_ptr(mvp));
            torus_shader.set_uniform("u_tex1", int(0));
            torus_shader.set_uniform("u_tex2", int(1));
            torus_shader.set_uniform("u_tex3", int(2));
            torus_shader.set_uniform("u_height_map", int(3));
            torus_shader.set_uniform("u_model", glm::value_ptr(model));
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grass);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rock);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, sand);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, height_map);

            glDepthMask(GL_TRUE);


            glEnable(GL_DEPTH_TEST);
            glColorMask(1, 1, 1, 1);
            glDepthFunc(GL_LEQUAL);
            torus.render();
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
         }

         // ufo 
         {
            auto model = glm::scale(glm::vec3(1, 1, 1) * 0.003f);
            auto axis = glm::cross(glm::vec3(0, 1, 0), tex_normal);
            auto color = glm::vec3(32 / 255.0, 32 / 255.0, 32 / 255.0);
            auto angle = glm::acos(glm::dot(glm::vec3(0, 1, 0), tex_normal));
            auto ufo_view = view 
               * glm::translate(glm::mat4(1.0), tex_point + tex_normal * 0.01f) 
               * glm::rotate(glm::mat4(1.0), (float) angle, axis);
            auto mvp = projection * ufo_view * model;
           
            ufo_shader.use();
            ufo_shader.set_uniform("u_color", color.x, color.y, color.z);
            ufo_shader.set_uniform("u_normal", normal.x, normal.y, normal.z);
            ufo_shader.set_uniform("u_mvp", glm::value_ptr(mvp));
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, height_map);

            glDepthMask(GL_TRUE);


            glColorMask(1, 1, 1, 1);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            ufo->draw();
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
         }


         // Generate gui render commands
         ImGui::Render();

         // Execute gui render commands using OpenGL backend
         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

         // Swap the backbuffer with the frontbuffer that is used for screen display
         glfwSwapBuffers(window);
      }

      // Cleanup
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();

      glfwDestroyWindow(window);
      glfwTerminate();
   }
   catch (std::exception const & e)
   {
      spdlog::critical("{}", e.what());
      return 1;
   }

   return 0;
}
