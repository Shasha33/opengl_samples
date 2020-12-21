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

bool is_pressed = 0;
float zoom = 1;
double saved_position[] = { 0.0, 0.0 };
double translation[] = { 0.0, 0.0 };
double window_h = 1280, window_w = 720;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f,  3.0f);

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

   if (translation[1] > glm::half_pi<double>()) {
      translation[1] = glm::half_pi<double>();
   } else if (translation[1] < -glm::half_pi<double>()) {
      translation[1] = -glm::half_pi<double>();
   }

   cameraPos.x = cos(translation[0]) * cos(translation[1]);
   cameraPos.y = sin(translation[1]);
   cameraPos.z = sin(translation[0]) * cos(translation[1]);
   cameraPos *= zoom;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   zoom += yoffset / window_h * 2;
   
   cameraPos.x = cos(translation[0]) * cos(translation[1]);
   cameraPos.y = sin(translation[1]);
   cameraPos.z = sin(translation[0]) * cos(translation[1]);
   cameraPos *= zoom;
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
   stbi_set_flip_vertically_on_load(false);

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
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

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

      auto cat = create_model("assets/fat_cat.obj");

      GLuint vbo, vao, ebo;
      create_skybox(vbo, vao, ebo);

      unsigned int cubemap = cubemap_texture();

      // init shader
      shader_t cube_shader("assets/simple-shader.vs", "assets/simple-shader.fs");
      shader_t cat_shader_reflect("assets/model.vs", "assets/model_reflect.fs");
      shader_t cat_shader_refract("assets/model.vs", "assets/model_refract.fs");
      shader_t cat_shader("assets/model.vs", "assets/model.fs");

      // Setup GUI context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO();
      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init(glsl_version);
      ImGui::StyleColorsDark();

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
         ImGui::Begin("Object settings");

         static float ratio = 1.33;
         ImGui::SliderFloat("refraction coef", &ratio, 1, 3);

         static bool reflection = 0;
         ImGui::Checkbox("only reflection", &reflection);

         static bool refraction = 0;
         ImGui::Checkbox("only refraction", &refraction);

         ImGui::End();

         glColorMask(1,1,1,1);
         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         auto view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
         auto projection = glm::perspective<float>(glm::radians(90.0), (float)display_w / display_h, 0.1, 100);

         // Render main
         {
            auto mvp = projection * glm::mat4(glm::mat3(view));
            
            glViewport(0, 0, display_w, display_h);
            
            cube_shader.use();
            cube_shader.set_uniform("u_mvp", glm::value_ptr(mvp));

            glBindVertexArray(vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
         }
         
         // Render offscreen
         {
            auto model = glm::scale(glm::vec3(1, 1, 1) * 0.03f);
            auto cat_mvp = projection * view * model;


            if (refraction) {
               cat_shader_refract.use();
               cat_shader_refract.set_uniform("u_mvp", glm::value_ptr(cat_mvp));
               cat_shader_refract.set_uniform("u_model", glm::value_ptr(model));
               cat_shader_refract.set_uniform("u_ratio", ratio);
               cat_shader_refract.set_uniform("u_camera_position", cameraPos.x, cameraPos.y, cameraPos.z);
            } else if (reflection) {
               cat_shader_reflect.use();
               cat_shader_reflect.set_uniform("u_mvp", glm::value_ptr(cat_mvp));
               cat_shader_reflect.set_uniform("u_model", glm::value_ptr(model));
               cat_shader_reflect.set_uniform("u_camera_position", cameraPos.x, cameraPos.y, cameraPos.z);
            } else {
               cat_shader.use();
               cat_shader.set_uniform("u_mvp", glm::value_ptr(cat_mvp));
               cat_shader.set_uniform("u_model", glm::value_ptr(model));
               cat_shader_refract.set_uniform("u_ratio", ratio);
               cat_shader.set_uniform("u_camera_position", cameraPos.x, cameraPos.y, cameraPos.z);
            }
            
            glDepthMask(GL_TRUE);

            glDisable(GL_DEPTH_TEST);
            glColorMask(0, 0, 0, 0);
            glDepthFunc(GL_LESS);
            cat->draw();

            glEnable(GL_DEPTH_TEST);
            glColorMask(1, 1, 1, 1);
            glDepthFunc(GL_LEQUAL);
            cat->draw();

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
