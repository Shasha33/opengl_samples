#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_texcoord;


struct vx_output_t
{
    vec3 position_world;
    vec2 texcoord;
    vec3 color_weight;
};
out vx_output_t v_out;

uniform sampler2D u_tex1;
uniform sampler2D u_tex2;
uniform sampler2D u_tex3;
uniform sampler2D u_height_map;

uniform int u_repeat;
uniform mat4 u_mvp;
uniform mat4 u_model;

void main()
{
  float texture_height =  texture(u_height_map, in_texcoord.xy).x;
  float height = texture_height / 2;
  v_out.position_world = (u_model * vec4(in_position + height * in_normal, 1.0)).xyz;
  v_out.texcoord = in_texcoord.xy * u_repeat;
  v_out.color_weight = normalize(vec3((1 - texture_height) * (1 - texture_height), (1 - texture_height) * texture_height, texture_height * texture_height));
  
  gl_Position = u_mvp * vec4(in_position + in_normal * height, 1.0);
}
