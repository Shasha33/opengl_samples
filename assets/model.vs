#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

uniform mat4 u_mvp;
uniform mat4 u_model;

struct vx_output_t
{
    vec3 position_world;
    vec3 normal;
};
out vx_output_t v_out;

void main()
{
  gl_Position = u_mvp * vec4(in_position, 1.0);

  v_out.position_world = (u_model * vec4(in_position, 1.0)).xyz; 
  v_out.normal = (u_model * vec4(in_normal, 0)).xyz; 
  // v_out.normal = in_normal;
}
