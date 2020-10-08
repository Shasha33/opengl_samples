#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_st;

struct vx_output_t
{
    vec3 st;
};

out vx_output_t v_out;

uniform mat4 u_mvp;

void main()
{

    v_out.st = in_position;

    gl_Position = u_mvp * vec4(in_position, 1.0);

}
