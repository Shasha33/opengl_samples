#version 330 core

layout (location = 0) in vec3 in_position;

struct vx_output_t
{
    vec3 position_world;
};
out vx_output_t v_out;

uniform mat4 u_mvp;
uniform mat4 u_model;

void main()
{
  gl_Position = u_mvp * vec4(in_position, 1.0);

  v_out.position_world = (u_model * vec4(in_position, 1.0)).xyz;
}
