#include <glm/glm.hpp>

#include <vector>
#include <utility>
#include <filesystem>
namespace fs = std::filesystem;

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <GL/glew.h>

#include <stb_image.h>
#include "torus.h"

void Torus::initializeMap(GLuint & texture, const char* filename) {
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
    
    if (!image) {
        printf("%s !\n", filename);
    }

    map = image;
    map_height = height;
    map_width = width;
}

float Torus::get_map_height(glm::vec2 tex) {
   unsigned char* pixel = map + int(tex.y * map_height) * map_width * 4 + int(tex.x * map_width) * 4;
   return pixel[0] / 255.0;
}

glm::vec3 Torus::get_map_point(float main_angle, float tube_angle) {
   glm::vec3 point = get_point(main_angle, tube_angle);
   glm::vec3 normal = get_normal(main_angle, tube_angle);
   glm::vec2 texture = get_texture(main_angle, tube_angle);
   
   float height = get_map_height(texture);

   return point + normal * height / 2.0f;
}

glm::vec3 Torus::get_map_normal(float main_angle, float tube_angle) {
   glm::vec3 right = get_map_point(main_angle + mainSegmentAngleStep, tube_angle);
   glm::vec3 left = get_map_point(main_angle - mainSegmentAngleStep, tube_angle);
   glm::vec3 top = get_map_point(main_angle, tube_angle + tubeSegmentAngleStep);
   glm::vec3 bot = get_map_point(main_angle, tube_angle - tubeSegmentAngleStep);
   
   glm::vec3 vec_x = right - left, vec_y = top - bot;

   return glm::normalize(glm::cross(vec_x, vec_y));
}

glm::vec3 Torus::get_point(float mainAngle, float tubeAngle) const {
        auto sinMainSegment = sin(mainAngle);
        auto cosMainSegment = cos(mainAngle);
        auto sinTubeSegment = sin(tubeAngle);
        auto cosTubeSegment = cos(tubeAngle);

        auto surfacePosition = glm::vec3(
            (_mainRadius + _tubeRadius * cosTubeSegment) * cosMainSegment,
            (_mainRadius + _tubeRadius * cosTubeSegment) * sinMainSegment,
            _tubeRadius * sinTubeSegment);    
        return surfacePosition;
}

glm::vec3 Torus::get_normal(float mainAngle, float tubeAngle) const {
        auto sinMainSegment = sin(mainAngle);
        auto cosMainSegment = cos(mainAngle);
        auto sinTubeSegment = sin(tubeAngle);
        auto cosTubeSegment = cos(tubeAngle);

        auto normal = glm::vec3(
                        cosMainSegment*cosTubeSegment,
                        sinMainSegment*cosTubeSegment,
                        sinTubeSegment
                    );   
        return normal;
}

glm::vec2 Torus::get_texture(float mainAngle, float tubeAngle) const {    
        return glm::vec2(tubeAngle / tubeSegmentAngleStep * tubeSegmentTextureStep,
             mainAngle / mainSegmentAngleStep * mainSegmentTextureStep);
}

void Torus::initializeData()
{
    auto currentMainSegmentAngle = 0.0f;
    auto currentMainSegmentTexCoordV = 0.0f;
    for (auto i = 0; i <= _mainSegments; i++) {
        auto currentTubeSegmentAngle = 0.0f;
        auto currentTubeSegmentTexCoordU = 0.0f;
        
        for (auto j = 0; j <= _tubeSegments; j++) {
            auto deltas = {std::make_pair(0, 0), std::make_pair(1, 0), std::make_pair(0, 1),
                         std::make_pair(0, 1), std::make_pair(1, 0), std::make_pair(1, 1)};
            for (auto d : deltas) {
                currentMainSegmentAngle = (i + d.first) * mainSegmentAngleStep;
                currentMainSegmentTexCoordV = (i + d.first) * mainSegmentTextureStep;
                currentTubeSegmentAngle = (j + d.second) * tubeSegmentAngleStep;
                currentTubeSegmentTexCoordU = (j + d.second) * tubeSegmentTextureStep;

                auto sinMainSegment = sin(currentMainSegmentAngle);
                auto cosMainSegment = cos(currentMainSegmentAngle);
                auto sinTubeSegment = sin(currentTubeSegmentAngle);
                auto cosTubeSegment = cos(currentTubeSegmentAngle);

                auto surfacePosition = glm::vec3(
                    (_mainRadius + _tubeRadius * cosTubeSegment)*cosMainSegment,
                    (_mainRadius + _tubeRadius * cosTubeSegment)*sinMainSegment,
                    _tubeRadius*sinTubeSegment);
                
                auto textureCoordinate = glm::vec3(currentTubeSegmentTexCoordU, currentMainSegmentTexCoordV, 0);
                
                auto normal = glm::vec3(
                        cosMainSegment*cosTubeSegment,
                        sinMainSegment*cosTubeSegment,
                        sinTubeSegment
                    );
                
                _vertices.push_back(surfacePosition);
                _vertices.push_back(normal);
                _vertices.push_back(textureCoordinate);
            
            }
        }
    }

    // for (int i = 0; i < _vertices.size(); i++) {
    //     printf("%f %f %f\n", _vertices[i].x, _vertices[i].y, _vertices[i].z);
    // }

    num_vertices_ = _vertices.size();
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _vertices.size(), &_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Torus::render() const
{
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
    glBindVertexArray(0);
}