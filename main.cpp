#pragma optimize("", off)

#include <iostream>
#include <vector>
#include <string>
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

// STB, load images
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// #include <learnopengl/camera.h>

#include "opengl_shader.h"

#include "obj_model.h"

// Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool is_pressed = 0;
float zoom = 1;
double saved_position[] = { 0.0, 0.0 };
double translation[] = { 0.0, 0.0 };
double window_h = 1280, window_w = 720;

static void glfw_error_callback(int error, const char *description)
{
   throw std::runtime_error(fmt::format("Glfw Error {}: {}\n", error, description));
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      is_pressed = 1;
      glfwGetCursorPos(window, saved_position, saved_position + 1);
   }
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      is_pressed = 0;
   }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
   if (is_pressed) {
      translation[0] += (saved_position[0] - xpos) / window_h; 
      translation[1] += (saved_position[1] - ypos) / window_w;
      saved_position[0] = xpos;
      saved_position[1] = ypos;
   }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   zoom += yoffset / window_h * 2;
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

   int width, height, nrChannels;
   std::vector<std::string> faces {
    "assets/MountainPath/posx.jpg",
    "assets/MountainPath/negx.jpg",
    "assets/MountainPath/posy.jpg",
    "assets/MountainPath/negy.jpg",
    "assets/MountainPath/posz.jpg",
    "assets/MountainPath/negz.jpg"
   };


   for (unsigned int i = 0; i < faces.size(); i++) {
      unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      // std::cout << (data != 0) << std::endl;
      stbi_image_free(data);
   }

   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

   return texture;
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

      glfwSetMouseButtonCallback(window, mouse_button_callback);
      glfwSetCursorPosCallback(window, cursor_position_callback);
      glfwSetScrollCallback(window, scroll_callback);

      if (glewInit() != GLEW_OK)
         throw std::runtime_error("Failed to initialize glew");

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      auto cat = create_model("assets/ball.obj");
      // render_target_t rt(window_h/2, window_w/2);

      GLuint vbo, vao, ebo;
      create_skybox(vbo, vao, ebo);

      unsigned int cubemap = cubemap_texture();

      // init shader
      shader_t cube_shader("assets/simple-shader.vs", "assets/simple-shader.fs");
      shader_t cat_shader("assets/model.vs", "assets/model.fs");

      // Setup GUI context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO();
      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init(glsl_version);
      ImGui::StyleColorsDark();

      auto const start_time = std::chrono::steady_clock::now();

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

         // GUI
         //ImGui::Begin("Triangle Position/Color");
         //static float rotation = 0.0;
         //ImGui::SliderFloat("rotation", &rotation, 0, 2 * glm::pi<float>());
         //static float translation[] = { 0.0, 0.0 };
         //ImGui::SliderFloat2("position", translation, -1.0, 1.0);
         //static float color[4] = { 1.0f,1.0f,1.0f,1.0f };
         //ImGui::ColorEdit3("color", color);
         //ImGui::End();

         auto model = glm::rotate(glm::mat4(1.0), glm::radians((float) translation[0] * 60), glm::vec3(0, 1, 0));
         // model =  glm::rotate(model, glm::radians((float) translation[1]), glm::vec3(1, 0, 0));
         auto view = glm::lookAt<float>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
         auto projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);


         // Render main
         {
            auto mvp = projection * view * model;

            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, display_w, display_h);

            glDepthMask(GL_FALSE);
            cube_shader.use();
            cube_shader.set_uniform("u_mvp", glm::value_ptr(mvp));

            glBindVertexArray(vao);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
         }
         
         // Render offscreen
         {
            auto cat_model = model;
            cat_model = model * glm::scale(glm::vec3(1, 1, 1) * (float) zoom * 0.01f);
            auto cat_mvp = projection * view * cat_model;


            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GEQUAL);

            cat_shader.use();
            cat_shader.set_uniform("u_mvp", glm::value_ptr(cat_mvp));
            cat_shader.set_uniform("u_model", glm::value_ptr(model));

            glm::vec3 light_dir = glm::rotateY(glm::vec3(1, 0, 0), glm::radians((float) translation[0] * 60));

            cat_shader.set_uniform<float>("u_color", 0.83, 0.64, 0.31);
            cat_shader.set_uniform<float>("u_light", light_dir.x, light_dir.y, light_dir.z);
            cat->draw();

            glDisable(GL_DEPTH_TEST);
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
