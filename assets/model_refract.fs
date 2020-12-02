#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec3 position_world;
    vec3 normal;
};

in vx_output_t v_out;

uniform vec3 u_color;
uniform vec3 u_light;
uniform mat4 u_view;

uniform vec3 u_camera_position;
uniform samplerCube u_tex;
uniform float u_ratio;

void main()
{
    vec3 I = normalize(u_camera_position - v_out.position_world);
    float coef = 1.0 / u_ratio;
    vec3 R = refract(I, normalize(v_out.normal), coef);
    o_frag_color = vec4(texture(u_tex, R).rgb, 1.0);
}
