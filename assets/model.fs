#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec3 position_world;
};

in vx_output_t v_out;

uniform vec3 u_color;
uniform vec3 u_light;

void main()
{
  vec3 vec_x = dFdx(v_out.position_world);
  vec3 vec_y = dFdy(v_out.position_world);

  vec3 normal = normalize(cross(vec_x, vec_y));

  float dot_val = 0.1 + 0.9 * clamp(dot(normal, u_light), 0, 1);

  o_frag_color = vec4(u_color * vec3(dot_val), 1);
}
