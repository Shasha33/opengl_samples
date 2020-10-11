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

uniform vec3 u_camera_position;
uniform samplerCube u_tex;
uniform float u_ratio;

void main()
{             
    vec3 I = normalize(v_out.position_world - u_camera_position);
    vec3 R = refract(I, normalize(v_out.normal), u_ratio);
    o_frag_color = vec4(texture(u_tex, R).rgb, 1.0);
}
