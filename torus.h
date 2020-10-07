//http://www.mbsoftworks.sk/tutorials/opengl4/011-indexed-rendering-torus/

#pragma once

class Torus {
public:
    Torus(int mainSegments, int tubeSegments, float mainRadius, float tubeRadius) : 
        _mainSegments(mainSegments), _tubeSegments(tubeSegments), _mainRadius(mainRadius), _tubeRadius(tubeRadius) {
            mainSegmentAngleStep = glm::radians(360.0f / float(_mainSegments));
            tubeSegmentAngleStep = glm::radians(360.0f / float(_tubeSegments));
            mainSegmentTextureStep = 1.0f  / float(_mainSegments);
            tubeSegmentTextureStep = 1.0f / float(_tubeSegments);
        }

    ~Torus() {
        if (map) stbi_image_free(map);
    }

    void render() const;
    void renderSpecial(int segments) const;

    float getMainRadius() const {
        return _mainRadius;
    };
    float getTubeRadius() const{
        return _tubeRadius;
    }

    void initializeData();
    void initializeMap(GLuint & texture, const char* filename);

    glm::vec3 get_point(float mainAngle, float tubeAngle) const;
    glm::vec2 get_texture(float mainAngle, float tubeAngle) const;
    glm::vec3 get_normal(float mainAngle, float tubeAngle) const;

    float get_map_height(glm::vec2 tex);
    glm::vec3 get_map_point(float main_angle, float tube_angle);
    glm::vec3 get_map_normal(float main_angle, float tube_angle);


    bool hasTextureCoordinates() const {
        return true;
    }

    bool hasNormals() const {
        return true;
    }

    bool hasPositions() const {
        return true;
    }

private:
    int _mainSegments;
    int _tubeSegments;
    float _mainRadius;
    float _tubeRadius;

    int _numIndices = 0;
    int num_vertices_ = 0;
    int _primitiveRestartIndex = 0;
    std::vector<glm::vec3> _vertices;

    GLuint _vbo, _vao;
    unsigned char* map;
    int map_height, map_width;

    float mainSegmentAngleStep;
    float tubeSegmentAngleStep;
    float mainSegmentTextureStep;
    float tubeSegmentTextureStep;
};