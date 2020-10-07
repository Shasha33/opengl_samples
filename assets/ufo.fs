#version 330 core

out vec4 o_frag_color;

uniform vec3 u_color;
uniform vec3 u_normal;
uniform mat4 u_mvp;

void main()
{
  o_frag_color = vec4(u_color, 1);
}