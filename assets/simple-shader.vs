#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

struct vx_output_t
{
    vec2 point;
};
out vx_output_t v_out;

uniform vec2 u_translation;
uniform vec2 u_scroll_center;
uniform float u_zoom;

void main()
{
    vec2 rotated_pos = in_position.xy;

    rotated_pos = (in_position.xy - u_scroll_center) * u_zoom + u_scroll_center + u_translation;

    v_out.point = in_position.xy;

    gl_Position = vec4(rotated_pos.x, rotated_pos.y, in_position.z, 1.0);
}
