#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec3 position_world;
    vec3 normal;
};
in vx_output_t v_out;

uniform vec3 u_camera_position;
uniform samplerCube u_tex;
uniform float u_ratio;


void main()
{
  vec3 I = normalize(v_out.position_world - u_camera_position);
  vec3 N = normalize(v_out.normal);
  vec3 Relf = reflect(I, N);
  vec3 reflect_color = texture(u_tex, Relf).rgb;

  float coef = 1.0 / u_ratio;
  vec3 R = refract(I, N, coef);
  vec3 refract_color = texture(u_tex, R).rgb;

  float cos = dot(-I, N);
  float R0 = ((1.0 - u_ratio) * (1.0 - u_ratio)) / ((1.0 + u_ratio) * (1.0 + u_ratio));
  float fresnel = R0 + (1 - R0) * pow((1 - cos), 5);

  o_frag_color = vec4(mix(refract_color, reflect_color, fresnel), 1.0);
}
