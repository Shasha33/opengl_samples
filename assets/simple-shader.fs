#version 330 core

out vec4 o_frag_color;

struct vx_output_t
{
    vec2 point;
};

in vx_output_t v_out;

uniform sampler1D u_tex;
uniform int u_iterations;
uniform vec2 u_c;

void main()
{
    vec2 z = v_out.point;

    int i;
    for(i = 0; i < u_iterations; i++) {
        float x = (z.x * z.x - z.y * z.y) + u_c.x;
        float y = (z.y * z.x + z.x * z.y) + u_c.y;

        if((x * x + y * y) > 3.0) break;
        z.x = x;
        z.y = y;
    }

    if (i == u_iterations) {
        o_frag_color = vec4(0, 0, 0, 1);    
    } else {
        o_frag_color = texture(u_tex, i / 15.0);
    }

}
