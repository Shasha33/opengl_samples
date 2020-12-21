#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec3 position_world;
    vec2 texcoord;
    vec3 color_weight;
    vec3 ufo_pos;
    vec3 ufo_normal;
};
in vx_output_t v_out;

uniform vec3 u_ufo_pos;
uniform vec3 u_ufo_normal;
uniform float u_ufo_range;

uniform sampler2D u_tex1;
uniform sampler2D u_tex2;
uniform sampler2D u_tex3;
uniform sampler2D u_height_map;

void main()
{
  vec3 light_dir = v_out.ufo_pos - v_out.position_world;
  float angle = dot(v_out.ufo_normal, normalize(light_dir));
  vec3 color = texture(u_tex1, v_out.texcoord).xyz * v_out.color_weight.x 
              + texture(u_tex2, v_out.texcoord).xyz * v_out.color_weight.y;
              + texture(u_tex3, v_out.texcoord).xyz * v_out.color_weight.z;
  if (length(light_dir) < u_ufo_range) o_frag_color = vec4(color, 1);
  else o_frag_color = vec4(color / 3, 1);
  
}