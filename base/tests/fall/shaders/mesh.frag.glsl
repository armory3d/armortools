#version 450

in vec2 tex_coord;
in vec3 normal;

out vec4 frag_color;

uniform sampler2D my_texture;

void main() {
    vec3 l = vec3(0.5, 0.0, 0.5);
    vec3 base_color = vec3(1.0, 1.0, 1.0); // texture(my_texture, tex_coord).rgb;
    vec3 ambient = base_color * 0.5;
    vec3 n = normalize(normal);
    float dotnl = max(dot(n, l), 0.0);
    vec3 diffuse = dotnl * base_color;
    frag_color = vec4(ambient + diffuse, 1.0);
}
