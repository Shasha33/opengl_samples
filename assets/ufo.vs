#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

uniform vec3 u_color;
uniform vec3 u_normal;
uniform mat4 u_mvp;

void main()
{
    gl_Position = u_mvp * vec4(in_position, 1.0);
}
